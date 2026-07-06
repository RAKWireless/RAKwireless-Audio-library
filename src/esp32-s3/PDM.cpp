#if defined(_VARIANT_RAK3112_)  || defined(ARDUINO_RAKWIRELESS_RAK3112)   //rak3112

#include "PDM.h"
#include <string.h>
#include "driver/i2s_common.h"
#include "driver/i2s_std.h"
#include "driver/i2s_pdm.h"
#include "esp_timer.h"

PDMClass      PDM;
I2SEventClass I2S;

// ------------------------------------------------------------------
// PDMClass
// ------------------------------------------------------------------

PDMClass::PDMClass()
  : _rx_chan(nullptr),
    _channel_open(false),
    _requested_bytes(BUFFER_SIZE),
    _effective_bytes(0),
    _dma_desc_num(4),
    _dma_frame_num(240),
    _requested_gain(0),
    _channels(1),
    _sample_rate(16000),
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

// Compute (dma_desc_num, dma_frame_num) so that
//   dma_desc_num * dma_frame_num * bytes_per_frame ≈ target_bytes.
// bytes_per_frame = channels * 2 (16-bit PDM only).
// dma_desc_num fixed at 4. Clamp dma_frame_num to IDF's 8..4092.
void PDMClass::_compute_dma_params_from_bytes(int target_bytes, int channels)
{
  const int bytes_per_frame = channels * 2;
  int desc = 4;
  int frames = target_bytes / (desc * bytes_per_frame);
  if (frames < 8)    frames = 8;
  if (frames > 4092) frames = 4092;
  _dma_desc_num  = desc;
  _dma_frame_num = frames;
  _effective_bytes = desc * frames * bytes_per_frame;
}

// Arduino nRF52 setGain range is roughly 0..80. IDF amplify_num is 1..15.
// Linear map with a small offset so 0 stays at 1 (no gain).
int PDMClass::_amplify_from_gain(int gain) const
{
  if (gain <= 0)  return 1;
  if (gain >= 80) return 15;
  return 1 + (gain * 14) / 80;
}

int PDMClass::bufferSize() const
{
  return _channel_open ? _effective_bytes : _requested_bytes;
}

bool PDMClass::setBufferSize(int size)
{
  if (_channel_open) return false;
  if (size <= 0)     return false;
  _requested_bytes = size;
  return true;
}

void PDMClass::setGain(int gain)
{
  _requested_gain = gain;
  if (_channel_open) {
    // Reprogram slot config live.
    i2s_pdm_rx_slot_config_t slot_cfg =
        I2S_PDM_RX_SLOT_PCM_FMT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT,
            _channels == 2 ? I2S_SLOT_MODE_STEREO : I2S_SLOT_MODE_MONO);
    slot_cfg.amplify_num = _amplify_from_gain(gain);
    i2s_channel_disable(_rx_chan);
    i2s_channel_reconfig_pdm_rx_slot(_rx_chan, &slot_cfg);
    i2s_channel_enable(_rx_chan);
  }
}

int PDMClass::begin(int channels, long sampleRate)
{
  if (_channel_open) return 0;

  _channels = channels;
  _sample_rate = sampleRate;
  _compute_dma_params_from_bytes(_requested_bytes, channels);

  i2s_chan_config_t chan_cfg = {
      .id = I2S_NUM_AUTO,
      .role = I2S_ROLE_MASTER,
      .dma_desc_num  = (uint32_t)_dma_desc_num,
      .dma_frame_num = (uint32_t)_dma_frame_num,
      .auto_clear = true,
      .auto_clear_before_cb = false,
      .allow_pd = false,
      .intr_priority = 0,
  };

  if (i2s_new_channel(&chan_cfg, nullptr, &_rx_chan) != ESP_OK) {
    Serial.println("PDM: i2s_new_channel failed");
    return 0;
  }

  i2s_pdm_rx_slot_config_t slot_cfg =
      I2S_PDM_RX_SLOT_PCM_FMT_DEFAULT_CONFIG(
          I2S_DATA_BIT_WIDTH_16BIT,
          channels == 2 ? I2S_SLOT_MODE_STEREO : I2S_SLOT_MODE_MONO);
  slot_cfg.amplify_num = _amplify_from_gain(_requested_gain);

  i2s_pdm_rx_clk_config_t clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG((uint32_t)sampleRate);

  i2s_pdm_rx_gpio_config_t gpio_cfg = {};
  gpio_cfg.clk = (gpio_num_t)42;
  gpio_cfg.din = (gpio_num_t)41;
  gpio_cfg.invert_flags.clk_inv = 0;

  i2s_pdm_rx_config_t pdm_cfg = {
      .clk_cfg  = clk_cfg,
      .slot_cfg = slot_cfg,
      .gpio_cfg = gpio_cfg,
  };
  if (i2s_channel_init_pdm_rx_mode(_rx_chan, &pdm_cfg) != ESP_OK) {
    Serial.println("PDM: init_pdm_rx_mode failed");
    i2s_del_channel(_rx_chan);
    _rx_chan = nullptr;
    return 0;
  }

  if (i2s_channel_enable(_rx_chan) != ESP_OK) {
    Serial.println("PDM: channel_enable failed");
    i2s_del_channel(_rx_chan);
    _rx_chan = nullptr;
    return 0;
  }

  _channel_open = true;
  _staging_pos = 0;
  _staging_avail = 0;
  _dma_events = _dropped_events = _user_callbacks = 0;
  return 1;
}

void PDMClass::end()
{
  _user_cb = nullptr;
  _stop_dispatch_task();

  if (_rx_chan) {
    i2s_channel_disable(_rx_chan);
    i2s_del_channel(_rx_chan);
    _rx_chan = nullptr;
  }
  _channel_open = false;
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
  if (room == 0 || _rx_chan == nullptr) return;

  size_t got = 0;
  esp_err_t err = i2s_channel_read(_rx_chan, _staging + _staging_avail,
                                   room, &got, 0);
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

// ---- Phase 4 callback plumbing (IRAM ISR path) ----

bool IRAM_ATTR pdm_isr_on_recv_trampoline(void *ctx);
static bool IRAM_ATTR pdm_isr_on_recv(i2s_chan_handle_t handle,
                                      i2s_event_data_t *event,
                                      void *user_ctx)
{
  (void)handle; (void)event;
  return pdm_isr_on_recv_trampoline(user_ctx);
}

bool IRAM_ATTR pdm_isr_on_recv_trampoline(void *ctx)
{
  static_cast<PDMClass *>(ctx)->_on_recv_isr();
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
      void (*cb)() = _user_cb;
      if (cb) {
        cb();
        _user_callbacks++;
      }
    }
  }
  TaskHandle_t self = _dispatch_task;
  _dispatch_task = nullptr;
  vTaskDelete(self);
}

bool PDMClass::_start_dispatch_task()
{
  if (_dispatch_task != nullptr) return true;
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
  xSemaphoreGive(_event_sem);
  while (_dispatch_task != nullptr) vTaskDelay(pdMS_TO_TICKS(2));
  vSemaphoreDelete(_event_sem);
  _event_sem = nullptr;
}

void PDMClass::onReceive(void (*cb)())
{
  if (_rx_chan == nullptr) return;

  if (cb == nullptr) {
    _user_cb = nullptr;
    _stop_dispatch_task();
    i2s_event_callbacks_t cbs = {};
    i2s_channel_disable(_rx_chan);
    i2s_channel_register_event_callback(_rx_chan, &cbs, nullptr);
    i2s_channel_enable(_rx_chan);
    return;
  }

  _user_cb = cb;
  if (!_start_dispatch_task()) return;

  i2s_event_callbacks_t cbs = {};
  cbs.on_recv = pdm_isr_on_recv;
  i2s_channel_disable(_rx_chan);
  i2s_channel_register_event_callback(_rx_chan, &cbs, this);
  i2s_channel_enable(_rx_chan);
}

// ------------------------------------------------------------------
// I2SEventClass
// ------------------------------------------------------------------

I2SEventClass::I2SEventClass()
  : _tx_chan(nullptr), _rx_chan(nullptr), _channel_open(false),
    _bclk(-1), _ws(-1), _dout(-1), _din(-1), _mclk(-1),
    _requested_bytes(BUFFER_SIZE), _effective_bytes(0),
    _dma_desc_num(4), _dma_frame_num(240),
    _rx_cb(nullptr), _tx_cb(nullptr),
    _rx_sem(nullptr), _tx_sem(nullptr),
    _rx_task(nullptr), _tx_task(nullptr),
    _rx_running(false), _tx_running(false),
    _rx_dma_events(0), _rx_dropped_events(0), _rx_user_callbacks(0),
    _tx_dma_events(0), _tx_dropped_events(0), _tx_user_callbacks(0)
{
}

I2SEventClass::~I2SEventClass() { endEvents(); }

void I2SEventClass::setPins(int8_t bclk, int8_t ws, int8_t dout,
                            int8_t din, int8_t mclk)
{
  _bclk = bclk; _ws = ws; _dout = dout; _din = din; _mclk = mclk;
}

void I2SEventClass::_compute_dma_params_from_bytes(int target_bytes,
                                                   i2s_slot_mode_t ch,
                                                   i2s_data_bit_width_t bits)
{
  int slots = (ch == I2S_SLOT_MODE_STEREO) ? 2 : 1;
  int bytes_per_frame = slots * ((int)bits / 8);
  if (bytes_per_frame < 1) bytes_per_frame = 2;
  int desc = 4;
  int frames = target_bytes / (desc * bytes_per_frame);
  if (frames < 8)    frames = 8;
  if (frames > 4092) frames = 4092;
  _dma_desc_num  = desc;
  _dma_frame_num = frames;
  _effective_bytes = desc * frames * bytes_per_frame;
}

int  I2SEventClass::bufferSize() const
{
  return _channel_open ? _effective_bytes : _requested_bytes;
}

bool I2SEventClass::setBufferSize(int size)
{
  if (_channel_open) return false;
  if (size <= 0)     return false;
  _requested_bytes = size;
  return true;
}

bool I2SEventClass::begin(i2s_mode_t mode, uint32_t rate,
                          i2s_data_bit_width_t bits_cfg,
                          i2s_slot_mode_t ch, int8_t slot_mask)
{
  (void)slot_mask;
  if (_channel_open) return false;
  if (mode != I2S_MODE_STD) {
    Serial.println("I2SEventClass: only I2S_MODE_STD is supported here");
    return false;
  }

  _compute_dma_params_from_bytes(_requested_bytes, ch, bits_cfg);

  i2s_chan_config_t chan_cfg = {
      .id = I2S_NUM_AUTO,
      .role = I2S_ROLE_MASTER,
      .dma_desc_num  = (uint32_t)_dma_desc_num,
      .dma_frame_num = (uint32_t)_dma_frame_num,
      .auto_clear = true,
      .auto_clear_before_cb = false,
      .allow_pd = false,
      .intr_priority = 0,
  };

  // Create BOTH tx and rx to mirror ESP_I2S's STD-mode behavior.
  if (i2s_new_channel(&chan_cfg, &_tx_chan, &_rx_chan) != ESP_OK) {
    Serial.println("I2S STD: i2s_new_channel failed");
    return false;
  }

  i2s_std_config_t std_cfg = {
      .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(rate),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(bits_cfg, ch),
      .gpio_cfg = {
          .mclk = (gpio_num_t)_mclk,
          .bclk = (gpio_num_t)_bclk,
          .ws   = (gpio_num_t)_ws,
          .dout = (gpio_num_t)_dout,
          .din  = (gpio_num_t)_din,
          .invert_flags = { .mclk_inv = 0, .bclk_inv = 0, .ws_inv = 0 },
      },
  };

  if (i2s_channel_init_std_mode(_tx_chan, &std_cfg) != ESP_OK) {
    Serial.println("I2S STD: init_std_mode (tx) failed");
    goto fail;
  }
  if (i2s_channel_init_std_mode(_rx_chan, &std_cfg) != ESP_OK) {
    Serial.println("I2S STD: init_std_mode (rx) failed");
    goto fail;
  }
  if (i2s_channel_enable(_tx_chan) != ESP_OK) goto fail;
  if (i2s_channel_enable(_rx_chan) != ESP_OK) goto fail;

  _channel_open = true;
  return true;

fail:
  if (_tx_chan) { i2s_del_channel(_tx_chan); _tx_chan = nullptr; }
  if (_rx_chan) { i2s_del_channel(_rx_chan); _rx_chan = nullptr; }
  return false;
}

bool I2SEventClass::end()
{
  endEvents();
  if (_tx_chan) {
    i2s_channel_disable(_tx_chan);
    i2s_del_channel(_tx_chan);
    _tx_chan = nullptr;
  }
  if (_rx_chan) {
    i2s_channel_disable(_rx_chan);
    i2s_del_channel(_rx_chan);
    _rx_chan = nullptr;
  }
  _channel_open = false;
  return true;
}

size_t I2SEventClass::write(const uint8_t *buffer, size_t size)
{
  if (_tx_chan == nullptr) return 0;
  size_t written = 0;
  esp_err_t err = i2s_channel_write(_tx_chan, buffer, size, &written,
                                    portMAX_DELAY);
  if (err != ESP_OK) return 0;
  return written;
}

size_t I2SEventClass::readBytes(char *buffer, size_t size)
{
  if (_rx_chan == nullptr) return 0;
  size_t got = 0;
  esp_err_t err = i2s_channel_read(_rx_chan, buffer, size, &got,
                                   portMAX_DELAY);
  if (err != ESP_OK) return 0;
  return got;
}

int I2SEventClass::available()
{
  if (_rx_chan == nullptr) return 0;
  return _effective_bytes;  // rough hint; matches ESP_I2S semantics
}

// ---- Phase 4 callback plumbing (identical pattern to PDM) ----

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
  i2s_event_callbacks_t cbs = {};
  if (_rx_cb) cbs.on_recv = i2sev_isr_on_recv;
  if (_tx_cb) cbs.on_sent = i2sev_isr_on_sent;

  bool ok = true;
  if (_rx_chan) {
    i2s_channel_disable(_rx_chan);
    if (i2s_channel_register_event_callback(_rx_chan, &cbs, this) != ESP_OK) ok = false;
    i2s_channel_enable(_rx_chan);
  }
  if (_tx_chan && _tx_chan != _rx_chan) {
    i2s_channel_disable(_tx_chan);
    if (i2s_channel_register_event_callback(_tx_chan, &cbs, this) != ESP_OK) ok = false;
    i2s_channel_enable(_tx_chan);
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
