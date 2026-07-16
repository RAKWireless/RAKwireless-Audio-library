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

### `library.properties`: `precompiled=true` disabled

The upstream `library.properties` sets `precompiled=true` and
`ldflags=-lDSpotterSDK`. There's no precompiled `.a` shipped for
esp32s3, so this was silently skipping the RAK3112 `.cpp` builds
entirely — RAK3112 changes had no effect on the built binary.
Disabling both flags lets the RAK3112 changes compile. **The
tradeoff**: DSPG voice-recognition examples (`VoiceRecognition_DSPG`,
`DSPG_PlayBack`) no longer link. Re-enable when shipping a DSPG
build, or gate them per-board.

Happy to split this into a `precompiled=` gated per architecture if
that's preferred.

### Verification

Every change has a dedicated verification sketch under
`examples/RAK3112/` (see README). Bench rig: RAK3112 core + RAK19007
base + RAK18003 interposer + RAK18030 PDM mic + RAK18060 stereo amp.

| Phase | Sketch | Result |
|---|---|---|
| baseline | AudioSmokeTest | tone / mic min/max/RMS across 8/16/22050/48 kHz |
| Phase 3 | Phase3_PDMAvailableCheck | grow/drain, canary, order — all PASS |
| Phase 4 | Phase4_CallbackCheck | 2000 events / 30 s, 0 drops, 11 µs dispatch |
| Phase 5 | Phase5_APIParityCheck | 500 Hz vs 67 Hz callback rate at 256/1920 B buffer, RMS climbs with gain |

## Test plan

- [ ] All `examples/RAK3112/*` build against Arduino-ESP32 v3.x
- [ ] Existing RAK4631 / RAK11310 / RAK11200 examples build unchanged
- [ ] 8 / 16 / 22 050 / 48 kHz playback all clean on RAK18060
- [ ] Full-duplex reproducer (PDMSerialPlotter `FULL_DUPLEX_TEST 1`)
      returns real mic samples while tone plays
- [ ] `PDM.read(buf, 64)` writes at most 64 bytes (canary test)
- [ ] DSPG examples flagged in CHANGELOG as gated by
      `precompiled=` — reviewers advise whether to re-enable per-arch

## Notes

- 96 kHz on RAK3112 is rate-supported but not hardware-verified in
  this PR.
- 8 kHz volume is slightly lower than 16 kHz — documented in the
  README + CHANGELOG under "Known open" with a workaround.
