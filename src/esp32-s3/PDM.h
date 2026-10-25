
#ifndef _PDM_H_INCLUDED
#define _PDM_H_INCLUDED

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "driver/i2s_std.h"
#include "driver/i2s_pdm.h"
#include "ESP_I2S.h"  // for i2s_mode_t enum + I2S_MODE_STD/PDM_RX

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1920
#endif

#ifndef PDM_STAGING_SIZE
#define PDM_STAGING_SIZE (2 * 1920)
#endif

// ------------------------------------------------------------------
// PDMClass — RAK3112 PDM microphone driver, raw ESP-IDF backed.
//
// Ports: this instance is pinned to I2S_NUM_0 (the only ESP32-S3 port
// that supports PDM_RX) and the library-wide `I2S` STD instance to
// I2S_NUM_1, so they run concurrently and can be started in either
// order. Other code that claims either port (e.g. the core's ESP_I2S
// I2SClass) will make the matching begin() fail.
//
// - Internal staging buffer, so available() reports real byte counts.
// - onReceive() callbacks are dispatched from a FreeRTOS task.
// - setBufferSize() sets the real IDF DMA sizing (call before begin()).
// - setGain() is applied in software (ESP32-S3 has no amplify_num).
// The driver owns its i2s_chan_handle_t directly instead of going
// through ESP_I2S's I2SClass, whose DMA sizing is fixed at 6 x 240.
//
// Public surface:
//   int  begin(channels, sample_rate)
//   void end()
//   int  available()        // bytes in staging
//   int  read(buf, size)    // at most size, order preserved
//   int  bufferSize()       // configured total DMA bytes (0 = default)
//   bool setBufferSize(int size)  // before begin()
//   void setGain(int gain)  // 0..80, software multiplier 1..15
//   void onReceive(void(*)())  // register/replace/detach
//   counters: dmaEvents(), droppedEvents(), userCallbacks(),
//             lastIsrMicros()
//
// Concurrency contract:
//   With a callback registered, consume samples from the callback
//   only. Staging is single-consumer, no internal lock.
class PDMClass
{
public:
  PDMClass();
  virtual ~PDMClass();

  int begin(int channels, long sampleRate);
  void end();

  int available();
  virtual int read(void* buffer, size_t size);

  // DMA sizing and gain.

  // Total DMA buffer size in bytes across all descriptors of the
  // RX channel. Reports the value currently in effect (or requested,
  // if begin() has not yet been called; 0 means the default).
  int bufferSize() const;

  // Request a new DMA buffer size in bytes. Must be called BEFORE
  // begin() — returns false and leaves state untouched if the
  // channel is already open. Clamped to the IDF-supported range.
  // On success, next begin() uses this size. Without it, begin() uses
  // ESP_I2S's default of 6 x 240 frames (90 ms at any rate), as 1.0.x did.
  bool setBufferSize(int size);

  // Software digital gain applied in read(). Accepts Arduino
  // nRF52-style 0..80; mapped internally to a 1..15 int multiplier
  // and applied per-sample to the staging output with int16 saturation.
  //
  // Why software and not hardware: the IDF's PDM RX slot has an
  // `amplify_num` field, but it's guarded by
  // SOC_I2S_SUPPORTS_PDM_RX_HP_FILTER — a capability ESP32-S3 does
  // NOT define. On this SoC the field literally doesn't exist. So
  // gain lives in software. Takes effect immediately regardless of
  // begin() state.
  void setGain(int gain);

  // Callbacks.
  void onReceive(void (*cb)());

  unsigned long dmaEvents()      const { return _dma_events; }
  unsigned long droppedEvents()  const { return _dropped_events; }
  unsigned long userCallbacks()  const { return _user_callbacks; }
  unsigned long lastIsrMicros()  const { return _last_isr_us; }

private:
  // Raw IDF channel handle for PDM RX.
  i2s_chan_handle_t _rx_chan;
  bool              _channel_open;

  // Requested / effective DMA sizing.
  int  _requested_bytes;      // user's setBufferSize() request
  int  _effective_bytes;      // what the IDF actually gave us
  int  _dma_desc_num;
  int  _dma_frame_num;

  // Gain state — software multiplier applied per-sample in read().
  int  _requested_gain;        // 0..80 arduino-style
  int  _gain_mult;             // derived 1..15 integer multiplier
  int  _channels;              // last begin() channels
  long _sample_rate;           // last begin() rate

  // Staging.
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

  friend bool pdm_isr_on_recv_trampoline(void *ctx);
  void _on_recv_isr();
  static void _dispatch_task_entry(void *ctx);
  void _dispatch_loop();
  bool _start_dispatch_task();
  void _stop_dispatch_task();

  // Helpers.
  void _compute_dma_params_from_bytes(int size, int channels);
  int  _amplify_from_gain(int gain) const;
};

extern PDMClass PDM;


// ------------------------------------------------------------------
// I2SEventClass — RAK3112 STD I2S output (and full-duplex RX)
// driver, raw ESP-IDF backed.
//
// Preserves the public API surface of the previous subclass-of-
// ESP_I2S::I2SClass so existing sketches (`I2S.setPins(...)`,
// `I2S.begin(I2S_MODE_STD, rate, bits, channels)`, `I2S.write(buf,
// size)`, `I2S.end()`) work unchanged. We just own the channels
// directly so setBufferSize() can control the DMA sizing that
// ESP_I2S hard-codes.
class I2SEventClass
{
public:
  I2SEventClass();
  ~I2SEventClass();

  // Pin setup — must be called before begin(). Matches ESP_I2S API.
  void setPins(int8_t bclk, int8_t ws, int8_t dout,
               int8_t din = -1, int8_t mclk = -1);

  // begin(mode, rate, bits_cfg, channels) — same signature as
  // ESP_I2S. Currently supports I2S_MODE_STD.
  bool begin(i2s_mode_t mode, uint32_t rate,
             i2s_data_bit_width_t bits_cfg,
             i2s_slot_mode_t ch, int8_t slot_mask = -1);

  bool end();

  size_t write(const uint8_t *buffer, size_t size);
  size_t readBytes(char *buffer, size_t size);
  int    available();

  // Raw channel-handle accessors (kept for source compatibility).
  i2s_chan_handle_t rxChan() { return _rx_chan; }
  i2s_chan_handle_t txChan() { return _tx_chan; }

  // DMA sizing.
  int  bufferSize() const;                      // effective bytes (0 = default)
  bool setBufferSize(int size);                 // before begin(); default 6 x 240 frames

  // Callbacks.
  void onReceive(void (*cb)());
  void onTransmit(void (*cb)());
  void endEvents();

  unsigned long rxDmaEvents()     const { return _rx_dma_events; }
  unsigned long rxDroppedEvents() const { return _rx_dropped_events; }
  unsigned long rxUserCallbacks() const { return _rx_user_callbacks; }
  unsigned long txDmaEvents()     const { return _tx_dma_events; }
  unsigned long txDroppedEvents() const { return _tx_dropped_events; }
  unsigned long txUserCallbacks() const { return _tx_user_callbacks; }

private:
  // Channel state.
  i2s_chan_handle_t _tx_chan;
  i2s_chan_handle_t _rx_chan;
  bool _channel_open;

  // Pin config.
  int8_t _bclk, _ws, _dout, _din, _mclk;

  // Requested / effective sizing.
  int _requested_bytes;
  int _effective_bytes;
  int _dma_desc_num;
  int _dma_frame_num;

  // 8 kHz BCLK-widening state. At sample_rate < 16 kHz we force the
  // ESP-IDF STD slot to 32-bit stereo so that BCLK lands at
  // 8000 * 32 * 2 = 512 kHz, the TAS2560 PLL's minimum input.
  //
  // If the caller asked for MONO at that rate, write() transparently
  // duplicates each mono sample into both L and R slots; the caller's
  // sketch keeps writing a mono buffer.
  bool _widen_slot;                 // true when we forced 32-bit stereo
  bool _mono_to_stereo_expand;      // true when caller asked mono but we
                                    // switched to stereo internally
  i2s_data_bit_width_t _user_bits;  // caller-requested bits (for expand)

  // Callback plumbing (same pattern as PDMClass).
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

  void _compute_dma_params_from_bytes(int size, i2s_slot_mode_t ch,
                                      i2s_data_bit_width_t bits);
};

extern I2SEventClass I2S;

#endif
