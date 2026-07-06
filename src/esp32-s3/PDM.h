
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
// library-wide global `I2S` is a *separate* `I2SClass`. Because
// PDM_RX is only supported on I2S_NUM_0, the IDF (via
// `id = I2S_NUM_AUTO`) assigns each instance to a different port.
//
// Phase 3 (driver_updates_plan.md): API foundation + real available()
//
// `ESP_I2S::available()` is deceptive — it returns a compile-time
// constant (`I2S_READ_CHUNK_SIZE`), not the actual DMA fill. So
// PDMClass keeps its own staging buffer, filled by non-blocking
// `i2s_channel_read` on demand:
//   - available()  reports the real fill of the staging buffer.
//   - read()       drains from staging first, then triggers a refill.
// Staging is small (`PDM_STAGING_SIZE`, two DMA descriptors), so
// latency addition is bounded (~120 ms at 16 kHz worst case, only if
// the caller lets the buffer sit full).
//
// I2SClass (upstream ESP_I2S, used directly as the library-wide `I2S`)
// still exposes its own `available()` / `readBytes()` / `write()` for
// STD-mode users — no wrapper needed on that side.

// Staging buffer size, in bytes. Two DMA descriptors' worth.
// Enough headroom for available() to show real accumulation across
// polling ticks, small enough to keep latency addition bounded.
#ifndef PDM_STAGING_SIZE
#define PDM_STAGING_SIZE (2 * 1920)
#endif

class PDMClass
{
public:
  PDMClass();
  virtual ~PDMClass();
  int begin(int channels, long sampleRate);
  void end();

  // Returns the number of *bytes* held in the internal staging
  // buffer, refilling non-blocking from the IDF DMA ring first.
  // Matches the Arduino nRF52 PDM library convention (BYTES, not
  // samples). Returns 0 before begin() or after end().
  int available();

  // Reads at most `size` bytes into `buffer`. Returns the number of
  // bytes actually written. Data is drained from the internal
  // staging buffer first; if that runs dry mid-call, we pull more
  // from the IDF non-blocking. Excess DMA bytes stay in staging (or
  // in the IDF ring) between calls, so repeated small reads preserve
  // sample order.
  virtual int read(void* buffer, size_t size);

  // Returns the configured DMA buffer length for this instance, in
  // bytes. Phase 3 reports the compile-time `BUFFER_SIZE` default
  // from `audio.h`. Phase 5 will allow this to diverge per-instance
  // via `setBufferSize()`.
  int bufferSize() const { return _buffer_size; }

private:
  I2SClass _pdm_i2s;
  int _buffer_size;

  // Staging buffer + cursors. Bytes at [_staging_pos .. _staging_avail)
  // are ready for read(); the rest of the buffer is free space.
  uint8_t _staging[PDM_STAGING_SIZE];
  size_t  _staging_pos;
  size_t  _staging_avail;

  // Non-blocking pull from the IDF into whatever room is free in the
  // staging buffer. Idempotent; safe to call on every available() /
  // read() entry.
  void _pump();
};

extern PDMClass PDM;
extern I2SClass I2S;

#endif
