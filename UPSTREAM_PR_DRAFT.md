# Draft upstream PR — RAK3112 audio path rewrite

Ready-to-copy body for the upstream PR against
`RAKwireless/RAKwireless-Audio-library`. Trim as appropriate.

---

## Title

`RAK3112: dual-port full duplex, per-rate TAS2560, event callbacks,
real DMA sizing`

## Summary

This is a from-scratch rewrite of the RAK3112 (ESP32-S3) audio path
against raw ESP-IDF `i2s_common` / `i2s_std` / `i2s_pdm`. RAK4631 /
RAK11310 / RAK11200 paths are unchanged.

Bug reports that motivated the work (bundled in the PR sender's
private tree; happy to attach separately if wanted):

- `TAS2560_8Khz_Not_working.md` — 8 kHz playback was silent because the
  static register table was 16/24-kHz-tuned and BCLK on this board is
  below TAS2560's PLL lock range.
- `Full_Duplex_RAK3112_problem.md` — PDM.begin() + I2S.begin(STD)
  torn each other down because both operated on a single shared
  `I2SClass`. `PDM.read()` also returned constant `0xFFFF`.
- `RAK3112_PDM_I2S_differences.md` — PDM/I²S API on RAK3112 was
  missing the Arduino nRF52 parity surface (`available`, `onReceive`,
  `onTransmit`, `setGain`, `setBufferSize`) used by cross-core sketches.

Six phases of work land here:

1. **TAS2560 per-rate support.** `TAS2560::begin(addr, rate)` and
   `set_sample_rate(fs)`. Per-rate register tables via
   `tas2560_dsp_coeffs_for_rate(fs)`. Includes a TI PPC3-generated
   table for ROM Mode 1 at 8 kHz (`registers_8khz[]`) and
   `CFG_META_BURST` opcode support in `transmit_registers`.
2. **Dual-port full duplex.** `PDMClass` and the library-wide
   `I2SEventClass I2S` own separate `i2s_chan_handle_t`s (PDM_RX on
   `I2S_NUM_0`, STD on `I2S_NUM_1`). Fixes `PDM.read` size bug too.
3. **`available()` foundation.** PDMClass gains a real staging
   buffer + non-blocking pull; `available()` reports genuine byte
   counts (ESP-IDF's own `available()` is a compile-time constant).
4. **Event callbacks.** `PDM.onReceive`, `I2S.onReceive` /
   `onTransmit` with FreeRTOS-task dispatch (ISR-safe → task
   context). Bench-verified 2000 DMA events / 30 s at 16 kHz, 0
   drops, dispatch latency 11 µs mean.
5. **`setBufferSize` + `setGain` + API parity.** Real IDF DMA
   sizing (was hardcoded 6×240 in ESP_I2S's `I2S_DEFAULT_CFG`).
   PDM software gain (ESP32-S3 lacks the IDF's `amplify_num`
   field).
6. **Regression sweep, versioning, README, CHANGELOG.**

### `library.properties` unchanged

`precompiled=true` / `ldflags=-lDSpotterSDK` stay as upstream has them.
An earlier draft of this work removed them, which broke all 14
CDSpotter voice examples on RAK4631 / RAK11200 / RAK11310. With
arduino-cli 1.5 the RAK3112 sources compile from source with the flag
set (there's no ESP32-S3 archive, so it only prints a "precompiled
library not found" note).

### TAS2560 fixes that affect all boards

`set_pcm_channel()` wrote ASI_FORMAT (0x14) instead of ASI_CHANNEL
(0x15), so channel selection never worked and `RightMode` set a 20-bit
word length; `set_unmute()` always powered up without boost; alarm
recovery reloaded a fixed register table. See CHANGELOG 2.0.1.

### Verification

Each phase was bench-tested with a dedicated test sketch (kept out of
the library; available on request). Bench rig: RAK3112 core + RAK19007
base + RAK18003 interposer + RAK18030 PDM mic + RAK18060 stereo amp.

| Phase | Sketch | Result |
|---|---|---|
| baseline | AudioSmokeTest_RAK18003 | tone / mic min/max/RMS across 8/16/22050/48 kHz |
| Phase 3 | PDM `available()` test sketch | grow/drain, canary, order — all PASS |
| Phase 4 | PDM callback test sketch | 2000 events / 30 s, 0 drops, 11 µs dispatch |
| Phase 5 | API parity test sketch | 500 Hz vs 67 Hz callback rate at 256/1920 B buffer, RMS climbs with gain |
| 2.0.1 | PlayBackFromSD with stereo WAVs | 8 / 16 / 44.1 / 48 / 96 kHz: correct speed (ratio 1.00), correct L/R, no hiss |
| sweep | compile, all four cores | no failures beyond those already in upstream 1.0.9 |

## Test plan

- [ ] All `examples/RAK3112/*` build against Arduino-ESP32 v3.x
- [ ] Existing RAK4631 / RAK11310 / RAK11200 examples build unchanged
- [ ] 8 / 16 / 44.1 / 48 / 96 kHz stereo playback clean on RAK18060,
      correct left/right (`PlayBackFromSD` / a stereo WAV per rate)
- [ ] Full-duplex reproducer (PDMSerialPlotter `FULL_DUPLEX_TEST 1`)
      returns real mic samples while tone plays
- [ ] `PDM.read(buf, 64)` writes at most 64 bytes (canary test)
- [ ] CDSpotter / DSPG examples still link on RAK4631 / RAK11200 /
      RAK11310

## Notes

- **Behaviour changes for existing sketches** are listed in the README
  section "Upgrading from 1.0.x". The one most likely to affect other
  boards: `set_pcm_channel()` now works, so a stream with audio in one
  slot only leaves the other amp silent (`LeftRightMode` restores the
  old mix). The RAK4631 / RAK11200 / RAK11300 `RightMode` examples
  haven't been re-tested on hardware.

- 96 kHz on RAK3112 uses the datasheet's "Mode 2 96k" (no DSP patch).
  In that mode the chip ignores ASI channel select, so the right amp
  selects its slot by inverting WCLK.
- Known limitations (`I2S.available()` hint, untested
  I²S callbacks and RAK18061) are listed in the README and CHANGELOG.
- `src/esp32-s3/PDM.zip` is removed. It held the pre-rewrite PDM
  library, which declares `I2S` with the old `I2SClass` type and would
  conflict if installed into the BSP as `doc/README(RAK3112).md` used
  to suggest.
- PDM and I²S use fixed ports (PDM on `I2S_NUM_0`, the only PDM-RX
  port; STD on `I2S_NUM_1`), so they start in either order.
