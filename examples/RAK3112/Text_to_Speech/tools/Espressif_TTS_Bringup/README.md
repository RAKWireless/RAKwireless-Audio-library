# Espressif Chinese TTS Bring-up (ESP-TTS / ESP-SR)

This folder contains the helper script for flashing Espressif TTS voice data
into the `voice_data` flash partition.

The Arduino sketch calls `lib/Espressif_TTS/esp_tts_bridge`, which wraps the
ESP-SR Chinese TTS APIs.

## Capabilities

| Item | Description |
|------|-------------|
| Language | Chinese only, UTF-8 input |
| Output | Synthesized at 16 kHz, resampled to the 22050 Hz playback pipeline |
| I2S | Stereo slot mode (`ch=2`) for the RAK18060 amplifier path |
| Voice data | `voice_data` flash partition at `0x400000`, 3 MB by default |
| Framework | Arduino ESP32 3.x with ESP-SR headers |

## Serial Commands

| Command | Behavior |
|---------|----------|
| `SAY:<text>` | Hybrid English path: SD phrase tier, SD word tier, then eSpeak fallback. |
| `SAYCN:<text>` | Force Espressif Chinese TTS for the whole text. |
| `SPEAK:<text>` | Force eSpeak synthesis for the whole text. |
| `STATUS` | Shows `espTTS=on/off`. |

## Arduino Build Setup

To enable Espressif Chinese TTS, run these commands in the sketch folder
`Text_to_Speech/`:


Windows (PowerShell):

```powershell
Copy-Item -Force build_opt_tts.h build_opt.h
Copy-Item -Force partitions_esp_tts.csv partitions.csv
```

macOS (bash/zsh):
```bash
cp build_opt_tts.h build_opt.h
cp partitions_esp_tts.csv partitions.csv
```

`build_opt.h` must contain only compile options, for example:

```text
-DTTS_ENABLE_ESPRESSIF
```

Do not add `//` comments to `build_opt.h`; Arduino may pass the line to the
compiler incorrectly.

## Voice Pack Selection (ESP32-S3 / RAK3112)

Espressif Chinese TTS is a compact offline synthesis engine. It is not as
natural as a cloud neural TTS voice, but different `.dat` voice packs can change
the tone.

| Voice | File | Size | ESP32-S3 Notes | Sound |
|-------|------|------|----------------|-------|
| **xiaole** | `esp_tts_voice_data_xiaole.dat` | ~2.8 MB | Recommended default | Female voice, relatively natural |
| xiaoxin_small | `esp_tts_voice_data_xiaoxin_small.dat` | ~2.8 MB | Requires `-DTTS_VOICE_XIAOXIN` | More mechanical |
| xiaoxin | `esp_tts_voice_data_xiaoxin.dat` | ~3.8 MB | Requires a larger `voice_data` partition | Standard xiaoxin |
| xiaoxin_custom | `esp_tts_voice_data_xiaoxin_custom.dat` | ~3.7 MB | Requires a larger `voice_data` partition | Customizable |

The current ESP32-S3 Arduino setup defaults to the `xiaole` voice template. If
you use a xiaoxin voice pack, add `-DTTS_VOICE_XIAOXIN` to `build_opt.h` and
make sure the `voice_data` partition is large enough.

For more natural Chinese prompts, generate 22.05 kHz WAV files on a PC with
Piper and place them in the SD card `/tts/phrases` or `/tts/words` folders. Use
`SAYCN:` only for text that should be synthesized directly on the device.

Official sample: [ESP-SR speech synthesis](https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/speech_synthesis/readme.html)

## Flash Voice Data

Run this from the project root or adjust the path as needed.

Windows (PowerShell):

```powershell
cd tools\Espressif_TTS_Bringup
.\flash_voice.ps1 -Port COM7 -Voice xiaole
```

macOS (bash/zsh):

```bash
cd tools/Espressif_TTS_Bringup
./flash_voice.sh --port /dev/tty.usbserial-XXXX --voice xiaole
```

Find the port with `ls /dev/tty.usbserial-*` (or `/dev/cu.usbserial-*`) while
the board is plugged in. The macOS script looks for `esptool` under
`~/Library/Arduino15/packages/esp32/tools/esptool_py/*/esptool` (installed
alongside the ESP32 board package in Arduino IDE).

Both scripts download the voice data if it is not already available locally,
then flash it to `0x400000`.

Manual flashing example:

```powershell
esptool.exe --chip esp32s3 -p COM7 write_flash 0x400000 data\esp_tts_voice_data_xiaole.dat
```

```bash
esptool --chip esp32s3 -p /dev/tty.usbserial-XXXX write_flash 0x400000 data/esp_tts_voice_data_xiaole.dat
```

## Verify

1. Reboot the board.
2. Send `STATUS`; it should show `espTTS=on`.
3. Send `SAYCN:<UTF-8 Chinese text>`.

## Relationship with SD + Piper

- Recorded English or prompt WAVs still use the SD phrase/word tiers.
- `SAYCN:` is for full-text Chinese synthesis through Espressif TTS.
- PC-side Piper can still be used to generate higher quality 22.05 kHz WAV
  assets for `/tts/phrases` and `/tts/words`.

## References

- [ESP-SR TTS documentation](https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/speech_synthesis/readme.html)
- [chinese_tts example](https://github.com/espressif/esp-skainet/tree/master/examples/chinese_tts)
