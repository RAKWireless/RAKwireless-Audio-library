# Changelog

## 2.0.0 — RAK3112 audio path rewrite; TAS2560 fixes for all boards

The RAK3112 (ESP32-S3) audio path is rewritten against raw ESP-IDF
`i2s_common` / `i2s_std` / `i2s_pdm`. It gains concurrent mic and
speaker (full duplex), runtime DMA sizing, event callbacks and per-rate
TAS2560 support. The TAS2560 amplifier driver, shared by every board,
gets several fixes from the datasheet (TI SLASE86E); where the datasheet
and the bench disagreed, the bench won and the code comment says so.

Bench-verified on RAK3112 + RAK19007 + RAK18003 + RAK18030 + RAK18060:
stereo WAVs at 8, 16, 22.05, 44.1, 48 and 96 kHz with correct speed and
left/right, no hiss, silent after playback; mic and speaker running
together in either start order at 8 and 16 kHz; recording to SD.

**Upgrading from 1.0.x:** some behaviour changed without a compile
error (amp channel select now works, `PDM.read()` no longer waits, and
more). See "Upgrading from 1.0.x — behaviour changes" in `README.md`.

### Breaking (RAK3112)

- **Global `I2S` type changed from ESP_I2S's `I2SClass` to
  `I2SEventClass`.** It reimplements the surface the examples use:
  `setPins`, `begin(mode, rate, bits, channels)`, `write`, `readBytes`,
  `available`, `end`, plus `setBufferSize` and the callbacks. Standard
  (Philips) mode only; ESP_I2S extras such as `playWAV` / `recordWAV`
  are gone. Sketches that declared their own `I2SClass I2S;` need to
  switch to `I2SEventClass`.
- **`PDM` and `I2S` are separate peripherals.** `PDM` used to be a
  wrapper around the global `I2S`; the mic is now reachable only
  through `PDM`.
- **`PDM.read(buf, size)`** returns the bytes that are ready, up to
  `size`, or 0; it no longer waits for a full block, and it no longer
  writes past `size`. Sketches must use the return value.
- **`audio.h`:** the `Stereo` macro is removed from the RAK3112 branch
  (unused there, and it collides with common variable names). Use
  `I2S_SLOT_MODE_STEREO` or `2`.

### Fixed (TAS2560, all boards)

- **`set_pcm_channel()` wrote the wrong register.** It wrote the
  channel mode into ASI_FORMAT (0x14) instead of ASI_CHANNEL (0x15), so
  channel selection never took effect (every amp played (L+R)/2) and
  `RightMode` set a 20-bit word length. Now writes ASI_CHANNEL.
- **ASI word length is set to 16 bits after every register-table
  load.** The chip resets to 24-bit and no table set it.
- **Mute.** `set_mute()` also zeroes the DAC gain, which silences the
  amp whatever state the DSP is in. `set_unmute()` restores the gain
  and the pre-mute power mode (it always powered up without boost).
  `set_mute()` is idempotent, and `set_gain()` while muted is
  remembered for `set_unmute()`.
- **Alarm recovery** (`clear_alarm()` / `clear_interrupt()`) reloads
  the register table for the configured rate and re-applies the gain,
  mute state and `set_pcm_channel()` mode.

### Added (TAS2560)

- **`begin(addr, sample_rate)`** (and a `TwoWire` variant): loads the
  register table for the rate (`tas2560_dsp_coeffs_for_rate()`), then
  programs PCM_RATE and the PLL for the BCLK the RAK3112 I²S produces.
  Needed for 8 kHz and 96 kHz.
- **`set_sample_rate(fs)`**: PCM_RATE (0x36), a 2-bit field: 8 kHz,
  16 kHz, 48 kHz or 96 kHz mode, with other rates mapped to the
  nearest. 16 kHz uses the 48 kHz mode, as the stock table does (the
  native 16 kHz mode with the stock DSP patch hissed).
- **`registers_8khz[]`**: a TI PPC3 export for 8 kHz (the stock
  table gave a constant whine there). `transmit_registers()` handles
  PPC3's `CFG_META_BURST` multi-byte writes.
- **`registers_96khz[]`**: the datasheet's "Mode 2 96k" (section
  9.5.5.3), no DSP patch. That mode ignores ASI channel select, so
  `set_pcm_channel(RightMode)` there selects the right slot by
  inverting WCLK.
- **PLL J** is 96 (49.152 MHz) at 16 kHz and below and 32 above. The
  datasheet's 1024 × Fs rule gave silence at 8 kHz; at 16 kHz, 16.384
  MHz made one amp hiss. See `apply_rate_dependent_init()`.

### Added (RAK3112)

- **Full duplex.** `PDM` (PDM RX) is pinned to `I2S_NUM_0`, the only
  ESP32-S3 port with PDM RX, and `I2S` (standard mode) to `I2S_NUM_1`,
  so they run at the same time and start in either order.
- **`setBufferSize(bytes)` / `bufferSize()`** on `PDM` and `I2S`: real
  IDF DMA sizing, set before `begin()`. The default is ESP_I2S's
  6 × 240 frames, as in 1.0.x.
- **`PDM.setGain(0..80)`**: a ×1–×15 software multiplier with int16
  saturation (ESP32-S3 has no hardware PDM gain). Not the same scale
  as the RAK4631's `setGain()`; see the README.
- **`PDM.available()`**: the number of bytes actually ready, from an
  internal staging buffer.
- **Callbacks:** `PDM.onReceive()`, `I2S.onReceive()` /
  `onTransmit()`. The DMA interrupt wakes a FreeRTOS task (core 0,
  priority 5) that calls the user function in task context; event
  counters are provided.
- **Low rates:** below 16 kHz, `I2S` uses 32-bit stereo slots so BCLK
  reaches the TAS2560 PLL's 512 kHz minimum; mono writes are
  duplicated into both slots.

### Changed (examples)

- **RAK3112 `PlayBackFromSD`:** 4 KB block reads and writes and a
  ~50 ms DMA buffer (the per-sample loop couldn't keep up at 96 kHz);
  left/right no longer swapped; mono files play on both amps; amps
  muted before `I2S.end()` (post-playback hiss); `begin(addr, rate)`
  per file; prints a playback-time ratio.
- **RAK3112 `RecordToSD`:** writes only the bytes `PDM.read()`
  returns, stops at the size in the WAV header, uses a 0.5 s mic
  buffer and `setGain(80)`; dropped the `#include "PDM.h"` that needed
  the old `PDM.zip`.
- **RAK3112 `PDMSerialPlotter_RAK18003`:** optional `FULL_DUPLEX_TEST`
  (off by default) plays a tone while plotting the mic.
- **RAK3112 HighRate plotters:** note that the RAK18032 can run at
  72000.
- **RAK4631 / RAK11200 examples that play a mono stream** set both
  amps to `LeftRightMode`, the (L+R)/2 mix every amp got in 1.0.x.
  With channel select now working, `LeftMode` / `RightMode` on a
  one-channel stream could leave a speaker silent. Stereo streams keep
  one channel per amp. RAK11300 examples already send mono in both
  slots and are unchanged. Compile-tested only.
- **`VoiceRecognition_DSPG` (RAK4631 / RAK11200 / RAK11300):** call
  `SetActiveCommandGroup()` and `begin(model, size)`, matching the
  current DSPG API (these didn't compile on 1.0.9).
- **RAK4631 `AnalogMicRecordToSD`** was a copy of the RAK11200 (ESP32)
  sketch and didn't compile; ported to the nRF52 SD and I2S APIs. Not
  bench-tested.
- **`dspg.cpp`:** the `tpt29555_1` / `tpt29555_2` expander objects are
  `static`, so sketches using those names link.

### Added (examples)

- **RAK3112 `AudioSmokeTest_RAK18003`:** plays a tone, records the
  mic and prints min / max / RMS; configurable rate and amp variant.

### Removed

- **`src/esp32-s3/PDM.zip`.** The old PDM library, which declares `I2S`
  with ESP_I2S's `I2SClass` and conflicts with the new drivers if
  installed into the BSP. Nothing needs installing: `audio.h` includes
  the right `PDM.h`.
- **RAK3112 `RAK1806X_test`, `RAK1806X_test_RAK18003`:** redundant
  amp demos (`PlayBack` and `AudioSmokeTest_RAK18003` cover them).
- **`examples/Test/`:** `Analog_Mic_Test` and `DSPG_Test` (stale API,
  didn't compile; covered by per-board examples), `AT_Test` and
  `FS_Test` (license-storage factory tests), `RAK18003_IO_Expander_Int`
  / `_Test` (every `*_RAK18003` example initialises the expanders).

### Known limitations

- PDM capture above about 75 kHz returns bad data: the RAK18032 mic's
  limit (72 kHz works).
- `I2S.available()` is a buffer-size hint, not a fill count.
- `I2S.onReceive()` / `onTransmit()` have not been bench-tested (the
  PDM callbacks have).
- The RAK18061 mono amp has not been tested.
- The RAK4631 / RAK11200 / RAK11300 example changes are compile-tested
  only.
