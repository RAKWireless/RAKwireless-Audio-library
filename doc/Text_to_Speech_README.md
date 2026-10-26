# WisBlock Text-to-Speech

A hybrid Text-to-Speech system built on RAKwireless WisBlock hardware. Text is
delivered to the device (Over USB Serial; later could be WiFi and/or LoRa) and
spoken aloud through a speaker.

The system uses a three-tier hybrid TTS approach:

1. **Phrase tier** - pre-recorded WAV files for common full sentences
   ("battery is low", "message received", "temperature is").
2. **Word tier** - pre-recorded WAV files for numbers, units, names, and
   frequently used nouns/verbs; concatenated on the fly.
3. **Fallback synthesis** - on-device speech synthesiser (eSpeak-NG) covers
   any word that is not in the phrase or word library, so any input produces
   *some* audio output.

Lookup order is 1 -> 2 -> 3. Tier 1 always wins because a full sentence recorded
in one take sounds more natural than concatenated words.

The Tier 3 fallback uses **eSpeak-NG** via the `arduino-espeak-ng` library.
**eSpeak-NG** was chosen, but others are possible - other on-device synthesiser
such as **ESP8266SAM** or **Talkie** could likely be used instead. Only the
`espeak_setup()` and `renderSpeak()` functions in the sketch are eSpeak-specific;
the rest of the pipeline just consumes the PCM they produce. Note eSpeak-NG is
GPL-licensed (see *Licensing* below).
For Chinese the Espressif TTS Speech Synthesis Model may be a good choice.

Note: The Espressif Chinese TTS can be optionally enabled using ENABLE_ESPRESSIF_TTS. The Chinese TTS enables Chinese TTS, but does note use hte Phase/Word tiers.

---

## Hardware

| Slot              | Module     | Function                                            |
|-------------------|------------|-----------------------------------------------------|
| Core              | RAK3112    | ESP32-S3 + SX1262 + WiFi/BLE                        |
| Base              | RAK19007   | USB-C, LiPo, 1 IO slot, 4 Sensor slots              |
| IO slot           | RAK18003   | Audio interposer; holds the microSD card            |
| On RAK18003       | RAK18060   | Stereo I2S amplifier (TAS2560 x2, 5.6 W)            |

A LiPo battery on the RAK19007 JST connector may be **required** when the amplifier
is active - USB alone may note be able to supply the peak current and the board will brown out. (this depends on the speaker being used)

Speaker: 4-8 Ohm on the RAK18060 output.

NOTE: Code is written for the RAK18060, however the RAK18061 could also be used (may be a better fit)

---

## Toolchain

- Arduino IDE 2.x
- Board package: `esp32 by Espressif Systems`, **version 3.3.x**
  (2.x is not supported - `ESP_I2S.h` is required)
  (boards manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`)
- Board selection: **RAKwireless RAK3112**
- USB CDC On Boot: **Enabled**
- PSRAM: **OPI PSRAM** enabled
- Partition Scheme:
  - **eSpeak-only build**: `Default 16MB (6.25MB APP / 3.43MB SPIFFS)` -
    remove any `partitions.csv` from the sketch folder for this mode.
  - **With Espressif Chinese TTS**: use the custom scheme (see
    *Enabling Espressif Chinese TTS* below); Arduino auto-selects it when
    `partitions.csv` is present in the sketch folder.
- Library: **RAKwireless-Audio-library** (install via Library Manager)
- eSpeak fallback libraries (git clone into `~/Documents/Arduino/libraries`):
  - `arduino-espeak-ng`   https://github.com/pschatzmann/arduino-espeak-ng
  - `arduino-posix-fs`    https://github.com/pschatzmann/arduino-posix-fs
  - `arduino-audio-tools` https://github.com/pschatzmann/arduino-audio-tools
  - `arduino-espeak-ng` wraps eSpeak-NG, which is **GPL** - see *Licensing*.
- Optional library for Chinese TTS: **Espressif_TTS** (shipped with this
  project under `examples/RAK3112/Text_to_Speech/Espressif_TTS/`; see
  *Enabling Espressif Chinese TTS* below).

### Regenerate the eSpeak voice data (REQUIRED)

The `espeak-ng-data` bundled inside `arduino-espeak-ng` is **stale and buggy**.
Symptom: words containing the letter pair `st` (test, best, stop, sister, ...)
are mis-synthesised - eSpeak converts `st` into the Unicode ST-ligature
(U+FB06) and spells the word out as garbage instead of pronouncing it.

Fix - replace the bundled data with current official data and regenerate the
PROGMEM headers. `examples/RAK3112/Text_to_Speech/tools/regen_data.py` (in this
repository) is a standalone
helper that runs `xxd` over every file in `espeak-ng-data/` to rebuild
`src/data/**/*.h` and `src/espeak-ng-data.h` (it does what `setup.py`'s
`create_data()` does, without the obsolete `command` module). It must be
run from the `arduino-espeak-ng` library root - `xxd` derives the C symbol
names from the file paths. Below, replace `<repo>` with the path where you
cloned this repository.

**macOS**

```bash
# 1. Get current, correct espeak-ng data
brew install espeak-ng

# 2. Replace the library's stale data directory
cd ~/Documents/Arduino/libraries/arduino-espeak-ng
rm -rf espeak-ng-data
cp -R "$(brew --prefix)/share/espeak-ng/espeak-ng-data" espeak-ng-data

# 3. Copy in the regenerator and run it
cp <repo>/examples/RAK3112/Text_to_Speech/tools/regen_data.py .
python3 regen_data.py
```

**Windows** - run the shell steps in **Git Bash**, which provides the `xxd`
tool `regen_data.py` needs (it ships with Git for Windows):

1. Install eSpeak NG from the official releases page - the `.msi` installer at
   <https://github.com/espeak-ng/espeak-ng/releases>. It installs the data to
   `C:\Program Files\eSpeak NG\espeak-ng-data`.

2. In Git Bash (note Git Bash addresses `C:\` as `/c/`):


```bash
# Replace the library's stale data directory
cd ~/Documents/Arduino/libraries/arduino-espeak-ng
rm -rf espeak-ng-data
cp -R "/c/Program Files/eSpeak NG/espeak-ng-data" espeak-ng-data

# Copy in the regenerator and run it
cp <repo>/examples/RAK3112/Text_to_Speech/tools/regen_data.py .
python regen_data.py
```

Requirements: a Python 3 install, plus `xxd` (standard on macOS; on Windows it
comes with Git Bash).

After regenerating, recompile the sketch. This step must be repeated whenever
the `arduino-espeak-ng` library is reinstalled or updated.

### Enabling Espressif Chinese TTS (optional)

The sketch has a compile-time toggle - `#define ENABLE_ESPRESSIF_TTS` in
`Text_to_Speech.ino` - that adds an Espressif Chinese TTS engine plus a
`SAYCN:` command. When the toggle is defined these steps must be followed:

1. **Install the `Espressif_TTS` library.** It ships in this repository under
   `examples/RAK3112/Text_to_Speech/Espressif_TTS/`. Copy that folder into
   your Arduino libraries dir:

   ```
   examples/RAK3112/Text_to_Speech/Espressif_TTS/  ->  ~/Documents/Arduino/libraries/Espressif_TTS/
   ```

   The final layout should look like:

   ```
   ~/Documents/Arduino/libraries/Espressif_TTS/esp_tts_bridge.h
   ~/Documents/Arduino/libraries/Espressif_TTS/esp_tts_bridge.cpp
   ~/Documents/Arduino/libraries/Espressif_TTS/library.properties
   ```

   Restart the Arduino IDE after copying.

2. **Activate the Espressif build options and partition table.** Run one of
   these commands in the sketch folder
   (`examples/RAK3112/Text_to_Speech/`):

   **macOS / Linux**
   ```bash
   cp -f build_opt_tts.h build_opt.h
   cp -f partitions_esp_tts.csv partitions.csv
   ```

   **Windows (PowerShell)**
   ```powershell
   Copy-Item -Force build_opt_tts.h build_opt.h
   Copy-Item -Force partitions_esp_tts.csv partitions.csv
   ```

   **Windows (cmd)**
   ```cmd
   copy /Y build_opt_tts.h build_opt.h
   copy /Y partitions_esp_tts.csv partitions.csv
   ```

   `build_opt.h` must contain **only compile switches** such as
   `-DENABLE_ESPRESSIF_TTS` - do not add `//` comments to it, the Arduino
   build passes the file directly to the compiler. When `partitions.csv` is
   present in the sketch folder the Arduino IDE automatically uses it as the
   custom partition table (no menu change needed).

3. **Flash the voice-data partition.** After the sketch itself is compiled
   and uploaded, flash the Chinese voice data with the helper in
   `examples/RAK3112/Text_to_Speech/tools/Espressif_TTS_Bringup/`:

   **macOS / Linux**
   ```bash
   cd examples/RAK3112/Text_to_Speech/tools/Espressif_TTS_Bringup
   ./flash_voice.sh --port /dev/tty.<your-port>
   ```

   **Windows (PowerShell)**
   ```powershell
   cd examples\RAK3112\Text_to_Speech\tools\Espressif_TTS_Bringup
   .\flash_voice.ps1 -Port COMx
   ```

To build **without** Espressif Chinese TTS: comment out the
`#define ENABLE_ESPRESSIF_TTS` line in `Text_to_Speech.ino`, remove any
`partitions.csv` from the sketch folder, empty out `build_opt.h`, and the
`Espressif_TTS` library does not need to be installed. `SAYCN:` will then be
reported as an unknown command.

---

## Project layout

```
RAKwireless-Audio-library/
  examples/RAK3112/Text_to_Speech/
    Text_to_Speech.ino       The Arduino sketch
    build_opt.h              Active Arduino build_opt include (edit or replace)
    build_opt_tts.h          Preset that turns on Espressif Chinese TTS
    build_opt_tts_xiaoxin.h  Preset for the "xiaoxin" Chinese voice
    partitions.csv           Active partition table (only used when present)
    partitions_esp_tts.csv   Preset partition table with a voice_data partition
    Recording_Guidelines.txt Notes on recording the WAV library
    Espressif_TTS/           Local Arduino library - copy to Arduino/libraries
                             to enable SAYCN (see Enabling Espressif Chinese TTS)
    tools/
      regen_data.py            Regenerates arduino-espeak-ng PROGMEM voice data
      TTS_WaveGen/             Piper-based WAV library generator (see its README)
      Espressif_TTS_Bringup/   Voice-data flashing helper (flash_voice.sh / .ps1)
  doc/
    Text_to_Speech_README.md This file
```

---

## SD card layout

The microSD card sits in the RAK18003 interposer and is read by the RAK3112
over SPI. Recommended layout:

```
/test.wav                     Optional standalone sanity-check file
/tts/
  phrases/                    Tier 1: full pre-recorded phrases
    all_clear.wav  battery_is_low.wav  message_received.wav
    temperature_is.wav  hello.wav  ...
  words/                      Tier 2: word bank, organised by part of speech
    numbers/      0.wav 1.wav ... 20.wav  30.wav ... 90.wav
                  hundred.wav  thousand.wav  point.wav  negative.wav
    units/        degrees.wav  celsius.wav  fahrenheit.wav  percent.wav
                  volts.wav  amps.wav  meters.wav  bytes.wav  ...
    nouns/        battery.wav  signal.wav  message.wav  alarm.wav  sensor.wav  ...
    verbs/        is.wav  are.wav  was.wav  has.wav  ask.wav  ...
    adjectives/   active.wav  automatic.wav  available.wav  bad.wav  busy.wav  ...
    adverbs/      always.wav  currently.wav  just.wav  locally.wav  ...
    pronouns/     i.wav  he.wav  her.wav  him.wav  it.wav  ...
    determiners/  a.wav  an.wav  all.wav  another.wav  any.wav  both.wav  ...
    common/       hello.wav  goodbye.wav  help.wav  yes.wav  no.wav  ...
    glue/         and.wav  the.wav  of.wav  at.wav  about.wav  after.wav  ...
  silence/
    pause_100ms.wav           Inserted between concatenated segments
  manifest.json               Index of all WAVs (loaded into RAM at boot)
```

WAV format: **22050 Hz, 16-bit, mono PCM** - and every file in the library
(phrases, words, silence) must use this exact format. 22050 Hz matches
eSpeak-NG's native output rate, so recorded and synthesized audio can share a
single I2S session with no resampling.

### Building the WAV library

There are two ways to produce the WAVs under `/tts/`:

- **Piper (quick, used during development)** - `tools/TTS_WaveGen/` synthesises
  the whole library from an Excel wordlist using
  [Piper](https://github.com/OHF-Voice/piper1-gpl) neural TTS, and also writes
  `manifest.json`. It is consistent, good quality and needs zero recording
  effort, which is why it was used to bring the system up. See that tool's
  own README.
- **Human voice (best quality)** - record the library with a real speaker.
  See `Recording_Guidelines.txt` for the recording discipline (one voice, one
  session, level-matched, trimmed) that keeps concatenated speech sounding
  natural and consistent.

Either way, every file must be 22050 Hz / 16-bit / mono PCM, and
`manifest.json` should index them (the device falls back to scanning
`/tts/phrases` if the manifest is absent).

Note: See preprocessText() in the code for the substitution of symbols into 
text (eg: % >> percent).
---

## Serial protocol

Connect at **115200 baud**, line ending = Newline. Command keywords are
**case-insensitive** (`say:`, `SAY:`, `SaY:` all work); arguments keep their
original case (SD paths and SAY/SPEAK text can be case-sensitive).

| Command         | Effect                                                              |
|-----------------|---------------------------------------------------------------------|
| `SAY:<text>`    | Speak text via the three-tier hybrid TTS (phrase -> word -> eSpeak).|
| `SAYCN:<text>`  | Force Espressif Chinese TTS for the whole text (needs the toggle enabled). |
| `SPEAK:<text>`  | Force eSpeak synthesis of the whole text (bypasses the wav library).|
| `PLAY:<path>`   | Play a single WAV file from SD. See path resolution below.          |
| `VOL:<0-21>`    | Set playback volume (0 = silent, 21 = max).                         |
| `RATE:<80-450>` | Set eSpeak speech rate in words per minute.                         |
| `LIST[:<dir>]`  | List the SD root, or a directory (e.g. `LIST:/tts/phrases`).        |
| `RELOAD`        | Re-read `/tts/manifest.json` and rebuild the phrase/word index.     |
| `HELP`          | Show the command help.                                              |
| `STATUS`        | Report ready + phrase/word counts + library source + eSpeak state.  |

### Path resolution for `PLAY:`

| Input                       | Resolves to                       |
|-----------------------------|-----------------------------------|
| `PLAY:hello`                | `/tts/phrases/hello.wav`          |
| `PLAY:test.wav`             | `/test.wav`                       |
| `PLAY:greetings/hi.wav`     | `/greetings/hi.wav`               |
| `PLAY:/tts/phrases/hi.wav`  | `/tts/phrases/hi.wav`             |

### Example: `SAY:Temperature is 23.5 degrees`

- `temperature is` -> phrase hit -> `/tts/phrases/temperature_is.wav`
- `23.5` -> number expansion -> `twenty` `three` `point` `five`
- each remaining token -> a word-bank WAV, or eSpeak synthesis if unknown
- all segments play back-to-back in one gapless 22050 Hz I2S session

---

## Known limitations

- Playback and synthesis are **blocking** - there is no `STOP` command and a
  command cannot be interrupted mid-utterance. Text typed during playback is
  buffered by the serial driver and processed when the current command ends.
- The amplifier draws current spikes; depending on the speaker, USB power
  alone can brown the board out. A LiPo on the RAK19007 JST connector fixes
  this (see Hardware).
- The whole audio pipeline is fixed at **22050 Hz, 16-bit, mono**. All recorded
  WAVs must match; mixed sample rates within one utterance are not supported.
- The eSpeak fallback voice is formant synthesis - intelligible but robotic,
  and audibly different from the Piper-recorded phrase/word tiers. This is by
  design: the fallback only ever speaks words with no recording, and sounding
  distinct flags which words those are.

---

## Troubleshooting

| Symptom                                    | Fix                                                                                                              |
|--------------------------------------------|------------------------------------------------------------------------------------------------------------------|
| `ESP_I2S.h: No such file`                  | Upgrade the ESP32 board package to 3.3.x.                                                                        |
| `WB_IO5` is not defined                    | Select **RAKwireless RAK3112** as the board.                                                                     |
| `esp_tts_bridge.h` is missing              | Copy `examples/RAK3112/Text_to_Speech/Espressif_TTS` into `~/Documents/Arduino/libraries/`, or comment out `#define ENABLE_ESPRESSIF_TTS`.    |
| `STATUS` reports `espTts=off` unexpectedly | Check that `build_opt.h`, `partitions.csv` and the flashed `voice_data` partition are all in place.              |
| `cannot specify '-o' with multiple files`  | Remove `//` comments from `build_opt.h`; keep one `-D...` switch per line.                                       |
| "Not enough space" during upload           | Use the 16 MB app partition scheme, or the custom `partitions_esp_tts.csv` when Espressif TTS is enabled.        |
| Boot loops / brownout when audio starts    | Amplifier current spike - connect a LiPo on the RAK19007 JST connector (see *Hardware*).                         |
| `st`-words spelled out as garbage          | Regenerate the eSpeak voice data - see *Regenerate the eSpeak voice data*.                                       |

---

## Licensing

- **eSpeak-NG**, and the `espeak-ng-data` it bundles, are licensed under the
  **GPL**. Because the eSpeak fallback is compiled into the firmware, the GPL
  applies to firmware built and distributed with it. If that is a problem for
  your use case, the Tier 3 fallback can be swapped for a non-GPL synthesiser
  (e.g. ESP8266SAM) or dropped entirely - the recorded phrase/word tiers do
  not depend on it.
- `examples/RAK3112/Text_to_Speech/tools/TTS_WaveGen/` uses **Piper** and per-voice
  neural models, each with
  its own licence. Check the licence of the Piper voice model you use before
  redistributing the WAVs it generates.
