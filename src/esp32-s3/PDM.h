
#ifndef _PDM_H_INCLUDED
#define _PDM_H_INCLUDED

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "ESP_I2S.h"

// Fallback BUFFER_SIZE for standalone PDM.h usage. `audio.h` defines
// BUFFER_SIZE = 1920 for the RAK3112 path and is the canonical source
// when present; this guarded default just keeps PDM.h compilable when
// included without audio.h.
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1920
#endif

#ifndef PDM_STAGING_SIZE
#define PDM_STAGING_SIZE (2 * 1920)
#endif

// Phase 2 — dual-port full duplex.
// Phase 3 — real available() via internal staging buffer.
// Phase 4 — Arduino-style event callbacks with FreeRTOS task dispatch.
//
// `PDMClass::onReceive(cb)` registers a callback that fires whenever
// the IDF signals a new DMA descriptor is ready. The IDF ISR posts to
// a FreeRTOS binary semaphore; a dedicated dispatch task (core 0,
// priority 5) takes the semaphore and calls `cb()` in task context.
// User code inside `cb()` may call any Arduino-safe API, including
// `PDM.read()` to drain the staging buffer.
//
// Concurrency contract:
//   - If you register onReceive(), consume samples FROM the callback.
//     Do NOT poll PDM.read() from the main loop while a callback is
//     registered — the staging buffer is single-consumer and has no
//     internal lock.
//   - Pass nullptr to onReceive() to detach; `end()` also detaches.
//   - Counters (`dmaEvents()`, `droppedEvents()`, `userCallbacks()`)
//     are volatile updates safe to sample from any core.
class PDMClass
{
public:
  PDMClass();
  virtual ~PDMClass();
  int begin(int channels, long sampleRate);
  void end();

  int available();
  virtual int read(void* buffer, size_t size);
  int bufferSize() const { return _buffer_size; }

  // Phase 4. Register or replace the receive callback. Pass nullptr
  // to detach. Must be called after begin() — the underlying IDF
  // channel handle is only valid then. Calling before begin() is a
  // no-op (the pointer is not remembered).
  void onReceive(void (*cb)());

  // Counters for verification / diagnostics. Volatile updates; snap
  // to a local before printing if you care about atomicity across
  // multi-word writes.
  unsigned long dmaEvents()      const { return _dma_events; }
  unsigned long droppedEvents()  const { return _dropped_events; }
  unsigned long userCallbacks()  const { return _user_callbacks; }
  // Timestamp (esp_timer_get_time / micros() domain) of the last
  // ISR fire. Subtract from micros() at user-callback entry to get
  // the driver-owned dispatch latency (ISR → task wakeup → user cb).
  unsigned long lastIsrMicros()  const { return _last_isr_us; }

private:
  I2SClass _pdm_i2s;
  int _buffer_size;

  uint8_t _staging[PDM_STAGING_SIZE];
  size_t  _staging_pos;
  size_t  _staging_avail;

  void _pump();

  // Callback plumbing.
  void (*_user_cb)();
  SemaphoreHandle_t _event_sem;
  TaskHandle_t      _dispatch_task;
  volatile bool     _dispatch_running;
  volatile unsigned long _dma_events;
  volatile unsigned long _dropped_events;
  volatile unsigned long _user_callbacks;
  volatile unsigned long _last_isr_us;

  // ISR entry points live at file scope in PDM.cpp so the IDF's
  // exact callback types don't leak into user-facing headers. They
  // call into these package-private helpers via user_ctx.
  friend bool pdm_isr_on_recv_trampoline(void *ctx);
  void _on_recv_isr();

  static void _dispatch_task_entry(void *ctx);
  void _dispatch_loop();
  bool _start_dispatch_task();
  void _stop_dispatch_task();
};

extern PDMClass PDM;

// Phase 4. The library-wide `I2S` global is now an `I2SEventClass`
// (subclass of ESP_I2S's `I2SClass`). Callers that only use base
// methods (`begin`, `setPins`, `write`, `readBytes`, `available`,
// `end`) see no behavior change — an `I2SEventClass` IS-A
// `I2SClass`. Add `I2S.onReceive(cb)` / `I2S.onTransmit(cb)` for
// event-driven use.
class I2SEventClass : public I2SClass
{
public:
  I2SEventClass();
  ~I2SEventClass();

  // RX (STD input) event — DMA descriptor ready.
  void onReceive(void (*cb)());
  // TX (STD output) event — DMA descriptor sent.
  void onTransmit(void (*cb)());

  unsigned long rxDmaEvents()     const { return _rx_dma_events; }
  unsigned long rxDroppedEvents() const { return _rx_dropped_events; }
  unsigned long rxUserCallbacks() const { return _rx_user_callbacks; }
  unsigned long txDmaEvents()     const { return _tx_dma_events; }
  unsigned long txDroppedEvents() const { return _tx_dropped_events; }
  unsigned long txUserCallbacks() const { return _tx_user_callbacks; }

  // Detach both callbacks and stop dispatch tasks. Safe to call
  // before or after the underlying I2SClass::end().
  void endEvents();

private:
  void (*_rx_cb)();
  void (*_tx_cb)();
  SemaphoreHandle_t _rx_sem;
  SemaphoreHandle_t _tx_sem;
  TaskHandle_t      _rx_task;
  TaskHandle_t      _tx_task;
  volatile bool     _rx_running;
  volatile bool     _tx_running;
  volatile unsigned long _rx_dma_events, _rx_dropped_events, _rx_user_callbacks;
  volatile unsigned long _tx_dma_events, _tx_dropped_events, _tx_user_callbacks;

  bool _register_callbacks();
  friend bool i2sev_isr_on_recv_trampoline(void *ctx);
  friend bool i2sev_isr_on_sent_trampoline(void *ctx);
  void _on_recv_isr();
  void _on_sent_isr();
  static void _rx_task_entry(void *ctx);
  static void _tx_task_entry(void *ctx);
  void _rx_loop();
  void _tx_loop();
};

extern I2SEventClass I2S;

#endif
