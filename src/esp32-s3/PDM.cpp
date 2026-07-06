#if defined(_VARIANT_RAK3112_)  || defined(ARDUINO_RAKWIRELESS_RAK3112)   //rak3112

#include "PDM.h"
#include <string.h>
#include "driver/i2s_common.h"
#include "esp_timer.h"

PDMClass    PDM;
I2SEventClass I2S;

// ---------------------------------------------------------------
// PDMClass
// ---------------------------------------------------------------

PDMClass::PDMClass()
  : _buffer_size(BUFFER_SIZE),
    _staging_pos(0),
    _staging_avail(0),
    _user_cb(nullptr),
    _event_sem(nullptr),
    _dispatch_task(nullptr),
    _dispatch_running(false),
    _dma_events(0),
    _dropped_events(0),
    _user_callbacks(0),
    _last_isr_us(0)
{
}

PDMClass::~PDMClass() {}

int PDMClass::begin(int channels, long sampleRate)
{
  _pdm_i2s.setPinsPdmRx(42, 41);
  if (!_pdm_i2s.begin(I2S_MODE_PDM_RX, sampleRate, I2S_DATA_BIT_WIDTH_16BIT,
                      (i2s_slot_mode_t)channels))
  {
    Serial.println("Failed to initialize I2S!");
    return 0;
  }
  _staging_pos = 0;
  _staging_avail = 0;
  _dma_events = _dropped_events = _user_callbacks = 0;
  return 1;
}

void PDMClass::end()
{
  // Detach user callback and stop dispatch task BEFORE tearing down
  // the IDF channel — that guarantees no ISR fires while we free
  // the semaphore.
  _user_cb = nullptr;
  _stop_dispatch_task();

  _pdm_i2s.end();
  _staging_pos = 0;
  _staging_avail = 0;
}

void PDMClass::_pump()
{
  if (_staging_pos == _staging_avail) {
    _staging_pos = 0;
    _staging_avail = 0;
  } else if (_staging_pos > 0) {
    size_t pending = _staging_avail - _staging_pos;
    memmove(_staging, _staging + _staging_pos, pending);
    _staging_pos = 0;
    _staging_avail = pending;
  }

  size_t room = sizeof(_staging) - _staging_avail;
  if (room == 0) return;

  i2s_chan_handle_t h = _pdm_i2s.rxChan();
  if (h == NULL) return;

  size_t got = 0;
  esp_err_t err = i2s_channel_read(h, _staging + _staging_avail, room,
                                   &got, 0);
  if (err != ESP_OK && err != ESP_ERR_TIMEOUT) got = 0;
  _staging_avail += got;
}

int PDMClass::available()
{
  _pump();
  return (int)(_staging_avail - _staging_pos);
}

int PDMClass::read(void* buffer, size_t size)
{
  _pump();
  size_t pending = _staging_avail - _staging_pos;
  if (pending == 0) return 0;
  size_t to_copy = (pending < size) ? pending : size;
  memcpy(buffer, _staging + _staging_pos, to_copy);
  _staging_pos += to_copy;
  return (int)to_copy;
}

// ---- Phase 4 callback plumbing ----

// File-scope ISR trampoline. IDF passes `user_ctx` (our PDMClass*)
// so we can route back into the instance without dragging IDF types
// into the header.
// IDF requires ISR callbacks live in IRAM — a flash-cache miss during
// an interrupt context would crash. Every function reachable from
// the IDF ISR path below is annotated IRAM_ATTR.
bool IRAM_ATTR pdm_isr_on_recv_trampoline(void *ctx);   // fwd decl of friend
static bool IRAM_ATTR pdm_isr_on_recv(i2s_chan_handle_t handle,
                                      i2s_event_data_t *event,
                                      void *user_ctx)
{
  (void)handle; (void)event;
  return pdm_isr_on_recv_trampoline(user_ctx);
}

bool IRAM_ATTR pdm_isr_on_recv_trampoline(void *ctx)
{
  PDMClass *self = static_cast<PDMClass *>(ctx);
  self->_on_recv_isr();
  return false;
}

void IRAM_ATTR PDMClass::_on_recv_isr()
{
  _dma_events++;
  _last_isr_us = (unsigned long)esp_timer_get_time();
  BaseType_t higher_woken = pdFALSE;
  BaseType_t ok = xSemaphoreGiveFromISR(_event_sem, &higher_woken);
  if (ok != pdTRUE) _dropped_events++;
  if (higher_woken == pdTRUE) portYIELD_FROM_ISR();
}

void PDMClass::_dispatch_task_entry(void *ctx)
{
  static_cast<PDMClass *>(ctx)->_dispatch_loop();
}

void PDMClass::_dispatch_loop()
{
  while (_dispatch_running) {
    if (xSemaphoreTake(_event_sem, pdMS_TO_TICKS(100)) == pdTRUE) {
      void (*cb)() = _user_cb;   // single-word read; no need to lock
      if (cb) {
        cb();
        _user_callbacks++;
      }
    }
  }
  // Signal end() we're leaving, then self-delete.
  TaskHandle_t self = _dispatch_task;
  _dispatch_task = nullptr;
  vTaskDelete(self);
}

bool PDMClass::_start_dispatch_task()
{
  if (_dispatch_task != nullptr) return true;   // already up
  if (_event_sem == nullptr) {
    _event_sem = xSemaphoreCreateBinary();
    if (_event_sem == nullptr) return false;
  }
  _dispatch_running = true;
  BaseType_t ok = xTaskCreatePinnedToCore(
      _dispatch_task_entry, "PDM_evt", 4096, this, 5, &_dispatch_task, 0);
  return ok == pdPASS;
}

void PDMClass::_stop_dispatch_task()
{
  if (_dispatch_task == nullptr) {
    if (_event_sem) { vSemaphoreDelete(_event_sem); _event_sem = nullptr; }
    return;
  }
  _dispatch_running = false;
  // Wake the task if it's blocked in xSemaphoreTake.
  xSemaphoreGive(_event_sem);
  // Wait for task to acknowledge exit (sets _dispatch_task=nullptr).
  while (_dispatch_task != nullptr) {
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  vSemaphoreDelete(_event_sem);
  _event_sem = nullptr;
}

void PDMClass::onReceive(void (*cb)())
{
  i2s_chan_handle_t h = _pdm_i2s.rxChan();
  if (h == nullptr) return;   // begin() not called yet

  if (cb == nullptr) {
    // Detach: clear user pointer, stop task. The IDF channel keeps
    // running — the caller can still poll PDM.read().
    _user_cb = nullptr;
    _stop_dispatch_task();
    // Unregister IDF callback so it stops incrementing counters.
    i2s_event_callbacks_t cbs = {};
    i2s_channel_disable(h);
    i2s_channel_register_event_callback(h, &cbs, nullptr);
    i2s_channel_enable(h);
    return;
  }

  // First time (or after detach): bring the dispatch task up and
  // register with IDF. IDF requires the channel be disabled during
  // register_event_callback.
  _user_cb = cb;
  if (!_start_dispatch_task()) return;

  i2s_event_callbacks_t cbs = {};
  cbs.on_recv = pdm_isr_on_recv;
  i2s_channel_disable(h);
  i2s_channel_register_event_callback(h, &cbs, this);
  i2s_channel_enable(h);
}

// ---------------------------------------------------------------
// I2SEventClass
// ---------------------------------------------------------------

I2SEventClass::I2SEventClass()
  : _rx_cb(nullptr), _tx_cb(nullptr),
    _rx_sem(nullptr), _tx_sem(nullptr),
    _rx_task(nullptr), _tx_task(nullptr),
    _rx_running(false), _tx_running(false),
    _rx_dma_events(0), _rx_dropped_events(0), _rx_user_callbacks(0),
    _tx_dma_events(0), _tx_dropped_events(0), _tx_user_callbacks(0)
{
}

I2SEventClass::~I2SEventClass()
{
  endEvents();
}

bool IRAM_ATTR i2sev_isr_on_recv_trampoline(void *ctx);
bool IRAM_ATTR i2sev_isr_on_sent_trampoline(void *ctx);

static bool IRAM_ATTR i2sev_isr_on_recv(i2s_chan_handle_t handle,
                                        i2s_event_data_t *event,
                                        void *user_ctx)
{
  (void)handle; (void)event;
  return i2sev_isr_on_recv_trampoline(user_ctx);
}
static bool IRAM_ATTR i2sev_isr_on_sent(i2s_chan_handle_t handle,
                                        i2s_event_data_t *event,
                                        void *user_ctx)
{
  (void)handle; (void)event;
  return i2sev_isr_on_sent_trampoline(user_ctx);
}

bool IRAM_ATTR i2sev_isr_on_recv_trampoline(void *ctx)
{
  static_cast<I2SEventClass *>(ctx)->_on_recv_isr();
  return false;
}
bool IRAM_ATTR i2sev_isr_on_sent_trampoline(void *ctx)
{
  static_cast<I2SEventClass *>(ctx)->_on_sent_isr();
  return false;
}

void IRAM_ATTR I2SEventClass::_on_recv_isr()
{
  _rx_dma_events++;
  BaseType_t hw = pdFALSE;
  BaseType_t ok = xSemaphoreGiveFromISR(_rx_sem, &hw);
  if (ok != pdTRUE) _rx_dropped_events++;
  if (hw == pdTRUE) portYIELD_FROM_ISR();
}
void IRAM_ATTR I2SEventClass::_on_sent_isr()
{
  _tx_dma_events++;
  BaseType_t hw = pdFALSE;
  BaseType_t ok = xSemaphoreGiveFromISR(_tx_sem, &hw);
  if (ok != pdTRUE) _tx_dropped_events++;
  if (hw == pdTRUE) portYIELD_FROM_ISR();
}

void I2SEventClass::_rx_task_entry(void *ctx) { static_cast<I2SEventClass *>(ctx)->_rx_loop(); }
void I2SEventClass::_tx_task_entry(void *ctx) { static_cast<I2SEventClass *>(ctx)->_tx_loop(); }

void I2SEventClass::_rx_loop()
{
  while (_rx_running) {
    if (xSemaphoreTake(_rx_sem, pdMS_TO_TICKS(100)) == pdTRUE) {
      void (*cb)() = _rx_cb;
      if (cb) { cb(); _rx_user_callbacks++; }
    }
  }
  TaskHandle_t self = _rx_task;
  _rx_task = nullptr;
  vTaskDelete(self);
}
void I2SEventClass::_tx_loop()
{
  while (_tx_running) {
    if (xSemaphoreTake(_tx_sem, pdMS_TO_TICKS(100)) == pdTRUE) {
      void (*cb)() = _tx_cb;
      if (cb) { cb(); _tx_user_callbacks++; }
    }
  }
  TaskHandle_t self = _tx_task;
  _tx_task = nullptr;
  vTaskDelete(self);
}

bool I2SEventClass::_register_callbacks()
{
  // Choose the correct channel handle. On PDM instances this class
  // isn't used (PDMClass owns its own I2SClass); here we're on the
  // STD path so rxChan() is for input, txChan() for output.
  //
  // We register once with whichever callbacks are currently wanted
  // (either or both). IDF replaces the whole callback set on each
  // register call, so we always pass the combined struct.
  i2s_event_callbacks_t cbs = {};
  if (_rx_cb) cbs.on_recv = i2sev_isr_on_recv;
  if (_tx_cb) cbs.on_sent = i2sev_isr_on_sent;

  // For a mixed-direction channel we still get one handle each; the
  // IDF requires calling register_event_callback per handle. STD
  // sketches in this library are TX-only via `I2S.write`, but future
  // full-duplex STD would populate both. Handle both cases.
  i2s_chan_handle_t rx = rxChan();
  i2s_chan_handle_t tx = txChan();
  bool ok = true;
  if (rx != nullptr) {
    esp_err_t err = i2s_channel_disable(rx);
    (void)err;   // may already be disabled
    if (i2s_channel_register_event_callback(rx, &cbs, this) != ESP_OK) ok = false;
    i2s_channel_enable(rx);
  }
  if (tx != nullptr && tx != rx) {
    esp_err_t err = i2s_channel_disable(tx);
    (void)err;
    if (i2s_channel_register_event_callback(tx, &cbs, this) != ESP_OK) ok = false;
    i2s_channel_enable(tx);
  }
  return ok;
}

void I2SEventClass::onReceive(void (*cb)())
{
  if (cb == nullptr) {
    _rx_cb = nullptr;
    if (_rx_task) {
      _rx_running = false;
      xSemaphoreGive(_rx_sem);
      while (_rx_task != nullptr) vTaskDelay(pdMS_TO_TICKS(2));
      vSemaphoreDelete(_rx_sem); _rx_sem = nullptr;
    }
    _register_callbacks();
    return;
  }

  _rx_cb = cb;
  if (_rx_task == nullptr) {
    _rx_sem = xSemaphoreCreateBinary();
    _rx_running = true;
    xTaskCreatePinnedToCore(_rx_task_entry, "I2S_rx_evt", 4096, this, 5, &_rx_task, 0);
  }
  _register_callbacks();
}

void I2SEventClass::onTransmit(void (*cb)())
{
  if (cb == nullptr) {
    _tx_cb = nullptr;
    if (_tx_task) {
      _tx_running = false;
      xSemaphoreGive(_tx_sem);
      while (_tx_task != nullptr) vTaskDelay(pdMS_TO_TICKS(2));
      vSemaphoreDelete(_tx_sem); _tx_sem = nullptr;
    }
    _register_callbacks();
    return;
  }

  _tx_cb = cb;
  if (_tx_task == nullptr) {
    _tx_sem = xSemaphoreCreateBinary();
    _tx_running = true;
    xTaskCreatePinnedToCore(_tx_task_entry, "I2S_tx_evt", 4096, this, 5, &_tx_task, 0);
  }
  _register_callbacks();
}

void I2SEventClass::endEvents()
{
  onReceive(nullptr);
  onTransmit(nullptr);
}

#endif
