# Changelog

## 2.0.0 — RAK3112 audio path rewrite

The RAK3112 (ESP32-S3) audio path is rewritten against raw ESP-IDF
`i2s_common` / `i2s_std` / `i2s_pdm` and gains real runtime sizing,
event callbacks, dual-port full duplex, and per-rate TAS2560 support.
RAK4631 / RAK11310 / RAK11200 code paths are unchanged. See
`README.md`'s "RAK3112 — extended API and fixes" section for the
user-facing usage and `phase_status.md` for the change history.

### Breaking

- **Global `I2S` type changed from `I2SClass` to `I2SEventClass`.**
  `I2SEventClass` is a standalone class (not a subclass of
  ESP_I2S's `I2SClass`) that reimplements the public surface
  examples rely on: `setPins`, `begin(mode, rate, bits, channels)`,
  `write`, `readBytes`, `available`, `end`. Existing sketches that
  use the global `I2S` unchanged; sketches that declared their own
  `I2SClass I2S;` will need to switch to `I2SEventClass`.
- **`library.properties`** no longer sets `precompiled=true` or
  `ldflags=-lDSpotterSDK`. No precompiled `.a` shipped for ESP32-S3
  so this was silently skipping the RAK3112 `.cpp` builds; disabling
  it lets the RAK3112 changes actually compile. DSPG examples
  (`VoiceRecognition_DSPG`, `DSPG_PlayBack`) will not link with
  those disabled — re-enable when shipping DSPG support.

### Added (RAK3112)

- **`TAS2560::begin(uint8_t addr, uint32_t sample_rate)`** — routes
  through per-rate register tables via
  `tas2560_dsp_coeffs_for_rate(fs)` and calls
  `apply_rate_dependent_init(fs)` to reprogram PLL / clock-error
  registers. TwoWire variant also added.
- **`TAS2560::set_sample_rate(uint32_t fs)`** — programs
  `PCM_RATE (0x36)` per rate.
- **`registers_8khz[]`** in `tas2560reg.cpp` — TI PPC3 export for
  ROM Mode 1 at 8 kHz. Provenance documented inline.
- **`CFG_META_BURST` opcode** — `TAS2560::transmit_registers` handles
  PPC3 multi-byte burst writes.
- **`I2SEventClass::onReceive(cb)` / `onTransmit(cb)`** — Arduino
  nRF52-style event callbacks. IDF ISR posts to a FreeRTOS
  semaphore; a dispatch task (core 0, priority 5) invokes the user
  callback in task context. Counters:
  `rxDmaEvents/rxDroppedEvents/rxUserCallbacks`, TX equivalents.
- **`PDMClass::onReceive(cb)`** — same pattern, single dispatch
  task per instance. Additional `lastIsrMicros()` accessor for
  measuring driver-owned dispatch latency.
- **`bool setBufferSize(int bytes)`** on `PDMClass` and
  `I2SEventClass` — controls real IDF DMA sizing before `begin()`.
  Returns `false` post-`begin()` and leaves state untouched. Total
  size clamped to the IDF range.
- **`int bufferSize()`** — reports effective bytes when the channel
  is open, else the pending request.
- **`void PDMClass::setGain(int gain)`** — software multiplier
  (Arduino-style 0..80, mapped to 1..15). Applied per-sample in
  `read()` with int16 saturation.
- **`int PDMClass::available()`** — returns actual staged bytes,
  filled by non-blocking `i2s_channel_read` on demand. Unlike
  ESP-IDF's own `available()` which returns a compile-time constant.
- **8 kHz BCLK widening.** At `rate < 16 kHz`, `I2SEventClass::begin`
  forces `slot_bit_width = 32` and `slot_mode = STEREO`, taking BCLK
  from 128 kHz to 512 kHz — inside TAS2560 PLL spec. Mono callers
  get transparent mono→stereo interleave inside `write()`.

### Changed (RAK3112)

- **Dual-port full duplex.** `PDMClass` owns a private
  `i2s_chan_handle_t` on `I2S_NUM_0`; `I2SEventClass I2S` runs on
  `I2S_NUM_1`. Previous behavior was to reconfigure a single shared
  peripheral — second `begin()` silently tore down the first.
- **`PDM.read(buf, size)`** honors `size`. Previously wrote a full
  DMA block (~1920 bytes) regardless of the caller's buffer length,
  silently overrunning small buffers. Excess data now stays in the
  IDF DMA ring for the next call.
- **`TAS2560::set_mute()`** additionally zeroes SPK_CTRL's gain
  field (caching prior gain to the `volume` member).
  `set_unmute()` restores from that cache. Belt-and-braces against
  transient PLL states.
- **`audio.h`** — `Stereo` macro removed from the RAK3112 branch
  (name collision with common user variable names; unused in
  RAK3112 code). `BUFFER_SIZE` remains as a source-compat default.

### Fixed (RAK3112)

- **8 kHz playback** now produces clean audio. Root causes: (1)
  stock `registers[]` is 16/24-kHz-tuned — the PPC3 8 kHz table
  replaces it; (2) ESP32-S3's 128 kHz BCLK was below TAS2560's PLL
  lock range — BCLK widening fixes it. See `phase_status.md`.
- **PCM_RATE encoding for 8 kHz corrected** — the value is `0x30`,
  not `0x12` (an earlier draft assumption from before the PPC3
  export).

### Known open

- 8 kHz output amplitude is slightly lower than 16 kHz (likely
  from PPC3's `SPK_CTRL (0x04) = 0x5F` encoding difference vs
  stock's `0x4F`, and/or the mono→stereo interleave path).
  Callers can compensate with `AMP.set_gain()`.
- 96 kHz is rate-supported but not yet hardware-validated.
