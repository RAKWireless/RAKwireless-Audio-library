# Changelog

## 2.0.1 — TAS2560 fixes from the datasheet; 96 kHz and stereo verified

Bench-verified on RAK3112 + RAK18060 with stereo WAVs at 8, 16, 44.1,
48 and 96 kHz: correct speed, correct left/right, no hiss, silent
after playback. PDM mic and I²S speaker verified running together in
either start order at 8 and 16 kHz. Register details are from the TAS2560 datasheet
(TI SLASE86E); where the datasheet and the bench disagreed, the bench
won and the code comment says so.

**Upgrading from 1.0.x:** some behaviour changed without a compile
error (amp channel select now works, `PDM.read()` no longer waits,
and more). See "Upgrading from 1.0.x —
behaviour changes" in `README.md` before upgrading.

### Fixed (TAS2560, all cores)

- **`set_pcm_channel()` wrote the wrong register.** It wrote the
  channel mode into ASI_FORMAT (0x14) instead of ASI_CHANNEL (0x15),
  so channel selection never took effect (both amps stayed at
  (L+R)/2) and `RightMode` set a 20-bit word length. Now writes
  ASI_CHANNEL.
- **ASI word length is set to 16 bits after every register-table
  load.** The chip resets to 24-bit and no table set it; left-channel
  playback only had the right length because of the bug above.
- **`set_unmute()` restores the pre-mute power mode.** It always
  wrote `PWR_CTRL_1 = 0x80` (power up *without* boost), turning the
  boost off for tables that run with it on. `set_mute()` is now
  idempotent: a second call no longer caches gain 0 and makes the
  next unmute restore silence.
- **Alarm recovery re-applies the channel mode.** After the register
  reload, `clear_alarm()` / `clear_interrupt()` restore the last
  `set_pcm_channel()` mode (stereo collapsed to (L+R)/2 otherwise, and
  at 96 kHz the right amp played the left channel).
- **`set_gain()` while muted** no longer un-silences the amp or gets
  lost: it records the gain, `get_gain()` reports it, and
  `set_unmute()` applies it.
- **Alarm recovery reloads the configured rate.** `clear_alarm()` and
  `clear_interrupt()` reloaded the upstream table regardless of the
  `begin(addr, rate)` rate, so a fault during 8 kHz playback
  reconfigured the amp for another rate. They also keep gain and mute
  state across the reload.

### Fixed (RAK3112)

- **Default DMA buffer matches 1.0.x.** 2.0.0 defaulted to 1920 bytes
  (10–60 ms depending on rate and channels, versus 90 ms in 1.0.x), so
  sketches with slow loops could drop audio. Without `setBufferSize()`,
  `PDM` and `I2S` now use ESP_I2S's 6 × 240 frames again.
- **PDM and I²S have fixed ports.** `PDMClass` now always uses
  `I2S_NUM_0` (the only ESP32-S3 port with PDM RX) and `I2SEventClass`
  `I2S_NUM_1`. Both used `I2S_NUM_AUTO`, so calling `I2S.begin()` before
  `PDM.begin()` could take port 0 and make the mic fail. They can now
  start in either order. A failed `begin()` prints which port was busy.
- **PCM_RATE encoding.** PCM_RATE (0x36) is a 2-bit field (datasheet
  Table 95): register value `0x30 | code`, with 0 = 8 kHz, 1 = 16 kHz,
  2 = 48 kHz, 3 = 96 kHz. The 2.0.0 values were guesses with the rate
  in the upper nibble. 16 kHz deliberately uses the 48 kHz code
  (`0x32`, as upstream did): the 16 kHz code with the upstream DSP
  patch hissed.
- **96 kHz playback.** New `registers_96khz[]` for the datasheet's
  "Mode 2 96k" (section 9.5.5.3): no DSP patch, Class-G boost. In that
  mode the chip ignores ASI channel select, so `set_pcm_channel(RightMode)`
  selects the right slot by inverting WCLK (ASI_CFG_1 bit 3) instead.
- **PLL J.** `J = 96` (PLL_CLK 49.152 MHz) at 16 kHz and below,
  `J = 32` above. 16 kHz previously ran at 16.384 MHz and one amp
  hissed during playback. The datasheet's 1024 × Fs rule and its
  PLL_LOWF rule were both tried at 8 kHz and gave silence; see the
  comment in `apply_rate_dependent_init()`.
- **8 kHz stereo reached only the left amp.** The 32-bit slot widening
  left the I²S WS width at 16 BCLKs, malforming the right-channel
  frame. WS width now matches the slot.

### Changed (examples)

- **`RAK3112/AudioSmokeTest` renamed `AudioSmokeTest_RAK18003`**, to
  make it clear the sketch needs the RAK18003 interposer.

- **`PDMSerialPlotter_RAK18003`** is back to its upstream defaults:
  16 kHz with the speaker off (`FULL_DUPLEX_TEST 0`). Its comments now
  say that with `FULL_DUPLEX_TEST 1` at 48 kHz and above the speaker
  breaks up, because printing every mic sample can't keep up.
- Example comments no longer refer to the fork's development phases.
- **`PlayBackFromSD`**: reads and writes 4 KB blocks instead of one
  sample frame per SD read and `I2S.write()` call (that loop ran at
  0.6× real time at 96 kHz, which sounded slow); ~50 ms I²S DMA buffer;
  left/right no longer swapped (and mono samples no longer pair-
  swapped); mono files play on both amps; amps muted before
  `I2S.end()` (stops post-playback hiss); full `begin(addr, rate)` per
  file; prints a playback-time ratio.

### Changed (RAK4631 / RAK11200 examples)

- Examples that play a **mono** I²S stream now set both amps to
  `LeftRightMode`, the (L+R)/2 mix every amp got in 1.0.x because
  `set_pcm_channel()` never took effect. With the channel fix,
  `LeftMode` / `RightMode` on a one-channel stream could leave a
  speaker silent. Stereo streams (and stereo WAVs in `PlayBackFromSD` /
  `VoiceRecognitionPlayMusic`) keep one channel per amp. Affected:
  RAK4631 `PlayBack`, `PlayBack22K`, `PlayBack48K`, `PlayBackFromSD`,
  `RAK1806X_test`, `RAK1806X_test_RAK18003`, `VoiceRecognitionL1_PlayBack`;
  RAK11200 `PlayBack`, `PlayBack22K`, `PlayBack48KHz`, `PlayBackFromSD`,
  `RAK1806X_test`, `RAK1806X_test_RAK18003`, `VoiceRecognitionL1_PlayBack`,
  `VoiceRecognitionPlayMusic`. RAK11300 examples already send mono in
  both slots and are unchanged. Compile-tested only.

### Fixed (examples that failed to compile, also on upstream 1.0.9)

- **`dspg.cpp` expander globals are now `static`.** `tpt29555_1` /
  `tpt29555_2` were public symbols, so any sketch using those names
  failed to link on every core (found with the
  `Test/RAK18003_IO_Expander_*` sketches, since removed).
- **`RAK3112/RecordToSD`**: dropped `#include "PDM.h"`, which only
  resolved with upstream's old `PDM.zip` installed. `audio.h` already
  includes the fork's PDM driver. The record loop also wrote a full
  1920-byte buffer per `PDM.read()` whatever it returned, which broke
  with the non-blocking `read()`: it now writes only the bytes read,
  stops at the size in the WAV header, and uses a 0.5 s mic buffer
  (`PDM.setBufferSize(16000)`) to ride out SD write stalls. It also
  sets `PDM.setGain(80)` (×15); without gain the recording was very
  quiet. Bench-verified 2026-10-25.
- **`RAK4631/RAK11200/RAK11300 VoiceRecognition_DSPG`**: call
  `begin(model, size)` after `SetActiveCommandGroup()`, as
  `DSPG_PlayBack` does; `begin()` lost its no-argument form.
- **`RAK4631/AnalogMicRecordToSD`** was the RAK11200 (ESP32) sketch.
  Ported to nRF52: Arduino `SD` library, `I2S.RxIRQCallBack()` +
  `I2S.start()`, stereo capture with the left channel written as a
  16 kHz mono WAV, 128 ms DMA blocks, and a lost-block count at the
  end. The old byte-wise "2× volume" shift (which corrupted samples)
  is gone. Not bench-tested.

### Removed

- **`src/esp32-s3/PDM.zip`.** It held the pre-2.0.0 PDM library, which
  declares `I2S` with the old `I2SClass` type and conflicts with the
  rewritten drivers if installed into the BSP. Nothing needs
  installing: `audio.h` includes the fork's `PDM.h`.
- **`examples/RAK3112/Phase3_PDMAvailableCheck`, `Phase4_CallbackCheck`,
  `Phase5_APIParityCheck`.** Development test sketches, not examples;
  `AudioSmokeTest` remains (now `AudioSmokeTest_RAK18003`, see below).
- **`examples/RAK3112/RAK1806X_test`, `RAK1806X_test_RAK18003`.**
  Redundant amp-config demos: `PlayBack` and `AudioSmokeTest_RAK18003`
  cover the RAK18060/1 setup.
- **`examples/Test/`** (the whole folder). Duplicates or not audio
  examples: `Analog_Mic_Test` (ESP32-only, stale API; each core's
  `AnalogMicSerialPlotter` covers it), `DSPG_Test` (stale API; duplicate
  of `VoiceRecognition_DSPG`), `AT_Test` and `FS_Test` (Cyberon license
  storage factory tests), and `RAK18003_IO_Expander_Int` / `_Test`
  (IO-expander bring-up; every `*_RAK18003` example initialises the
  expanders).

### Corrections to 2.0.0 notes

- The unwidened 8 kHz BCLK was 256 kHz (two slots per frame), not
  128 kHz.
- The 8 kHz PPC3 table runs Class-G, not Class-H.
- With BCLK in spec, `I2S.end()` alone does *not* silence the amps:
  mute first.

### Known open

- PDM capture above about 75 kHz returns bad data. That is the
  RAK18032 mic's limit, not the driver's; 72 kHz works.
- `I2S.available()` is a buffer-size hint, not a fill count.
- `I2S.onReceive()` / `onTransmit()` not bench-tested.
- RAK18061 mono amp not tested.
- `RAK4631/AnalogMicRecordToSD` and `*/VoiceRecognition_DSPG` compile
  but haven't been run on hardware.

## 2.0.0 — RAK3112 audio path rewrite

The RAK3112 (ESP32-S3) audio path is rewritten against raw ESP-IDF
`i2s_common` / `i2s_std` / `i2s_pdm` and gains real runtime sizing,
event callbacks, dual-port full duplex, and per-rate TAS2560 support.
RAK4631 / RAK11310 / RAK11200 code paths are unchanged. See
`README.md`'s "RAK3112 — extended API and fixes" section for the
user-facing usage.

### Breaking

- **Global `I2S` type changed from `I2SClass` to `I2SEventClass`.**
  `I2SEventClass` is a standalone class (not a subclass of
  ESP_I2S's `I2SClass`) that reimplements the public surface
  examples rely on: `setPins`, `begin(mode, rate, bits, channels)`,
  `write`, `readBytes`, `available`, `end`. Existing sketches that
  use the global `I2S` unchanged; sketches that declared their own
  `I2SClass I2S;` will need to switch to `I2SEventClass`.

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
  IDF DMA ring for the next call. `read()` also no longer waits: it
  returns what's ready, or 0. Sketches must use the return value.
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
  replaces it; (2) ESP32-S3's 8 kHz BCLK was below TAS2560's PLL
  lock range — BCLK widening fixes it.
- **PCM_RATE encoding for 8 kHz corrected** — the value is `0x30`,
  not `0x12` (an earlier draft assumption from before the PPC3
  export).

### Fixed (all cores)

- **Restore `precompiled=true` / `ldflags=-lDSpotterSDK` in
  `library.properties`.** An earlier 2.0.0 draft removed them, which
  left the bundled `libDSpotterSDK.a` (`src/esp32`, `src/cortex-m4`,
  `src/cortex-m0plus`) unlinked, so every CDSpotter voice example
  failed with `undefined reference to DSpotter_*` on RAK4631,
  RAK11200 and RAK11310. With arduino-cli 1.5 the RAK3112 sources still
  compile with the flag set (there is no `src/esp32s3` archive, so
  arduino-cli only prints a "precompiled library not found" note).

### Known open

- 8 kHz output amplitude is slightly lower than 16 kHz (likely
  from PPC3's `SPK_CTRL (0x04) = 0x5F` encoding difference vs
  stock's `0x4F`, and/or the mono→stereo interleave path).
  Callers can compensate with `AMP.set_gain()`.
