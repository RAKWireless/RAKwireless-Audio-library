
#ifndef _PDM_H_INCLUDED
#define _PDM_H_INCLUDED

#include <Arduino.h>
#include "ESP_I2S.h"

// Fallback BUFFER_SIZE for standalone PDM.h usage. `audio.h` defines
// BUFFER_SIZE = 1920 for the RAK3112 path and is the canonical source
// when present; this guarded default just keeps PDM.h compilable when
// included without audio.h.
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1920
#endif

// Phase 2 (driver_updates_plan.md): default port mapping on ESP32-S3
//
//   PDM  RX  ->  I2S_NUM_0   (ESP32-S3 only supports PDM_RX on port 0)
//   STD I/O ->  I2S_NUM_1
//
// The mapping is enforced by ownership, not by an explicit port id:
// `PDMClass` owns a private `I2SClass _pdm_i2s` instance, and the
// library-wide global `I2S` is a *separate* `I2SClass`. ESP_I2S calls
// `i2s_new_channel` with `id = I2S_NUM_AUTO`, so the IDF picks the
// next available port for each instance. Because PDM_RX is only
// supported on I2S_NUM_0, that instance lands there; the STD instance
// then lands on I2S_NUM_1. The two peripherals run concurrently — a
// later `I2S.begin(I2S_MODE_STD, …)` no longer tears down the PDM
// path the way it did when both classes shared one global `I2S`.
//
// Phase 3 (driver_updates_plan.md): API foundation.
//
// PDMClass exposes:
//   int available()   — bytes currently held in the IDF DMA ring,
//                       matching the Arduino nRF52 PDM convention
//                       (BYTES, not samples).
//   int read(buf,sz)  — at-most-sz; preserves order across calls
//                       (Phase 2).
//   int bufferSize()  — configured DMA buffer length for this
//                       instance, in bytes. Phase 3 ships the getter;
//                       Phase 5 will add a matching setBufferSize().
//                       Until then, the value tracks the BUFFER_SIZE
//                       compile-time default from `audio.h`.
//
// I2SClass (upstream ESP_I2S, used directly as the library-wide `I2S`)
// already provides:
//   int available()
//   size_t readBytes(buf, size)
//   size_t write(buf, size)
// — no wrapper needed. Phase 5 will add a setBufferSize() on the I2S
// side too; until then the IDF default (~6 * 240 = 1440 frames) holds.
class PDMClass
{
public:
  PDMClass();
  virtual ~PDMClass();
  int begin(int channels, long sampleRate);
  void end();

  // Returns the number of *bytes* currently available to read from
  // the PDM DMA ring. Matches the Arduino nRF52 PDM library
  // convention so cross-core ports do not need conditional sizing.
  // Returns 0 before begin() or after end().
  int available();

  // Reads at most `size` bytes into `buffer`. Returns the number of
  // bytes actually written. Excess DMA bytes are retained by the
  // ESP-IDF i2s_channel_read ring between calls, so repeated small
  // reads preserve sample order without dropping data.
  virtual int read(void* buffer, size_t size);

  // Returns the configured DMA buffer length for this instance, in
  // bytes. Phase 3 reports the compile-time `BUFFER_SIZE` default
  // from `audio.h`. Phase 5 will allow this to diverge per-instance
  // via `setBufferSize()`.
  int bufferSize() const { return _buffer_size; }

private:
  // Private I2S binding for the PDM_RX path. Constructed alongside
  // the global `I2S` below; the IDF assigns it I2S_NUM_0 because
  // PDM_RX is only supported there on ESP32-S3.
  I2SClass _pdm_i2s;
  // Per-instance buffer state (Phase 3 step 3). Defaults to the
  // compile-time BUFFER_SIZE macro from audio.h for backward
  // compatibility with the existing examples. Setter arrives in
  // Phase 5; do NOT try to make BUFFER_SIZE track this at compile
  // time — it is intentionally a per-instance runtime field now.
  int _buffer_size;
};

extern PDMClass PDM;
extern I2SClass I2S;

#endif
