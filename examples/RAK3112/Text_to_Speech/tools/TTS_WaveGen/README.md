# TTS Wave Generation

Generate the WAV library for the WisBlock TTS system using
[Piper](https://github.com/OHF-Voice/piper1-gpl) (GPL fork). Input is an
Excel sheet; output is a tree of 22.05 kHz / 16-bit / mono PCM WAVs ready
to be copied onto the microSD card under `/tts/`.

## Setup

```bash
cd tools/TTS_WaveGen
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Piper voice models are downloaded on first use into `voices/`.

## Quick start

Note: An inital wordlist.xlsx is provided, you can skip step #1 if desired.
 
```bash
# 1. Create a starter wordlist (only needed once)
python generate_waves.py --init

# 2. Edit wordlist.xlsx in Excel / Numbers / LibreOffice

# 3. Generate
python generate_waves.py
```

Output lands in `tts/<folder>/<filename>`, matching the SD layout in the
top-level `README.md` (e.g. `tts/phrases/hello.wav`,
`tts/words/numbers/0.wav`). Copy the contents of `tts/` to `/tts/` on
the microSD card.

Re-running skips files that already exist. Pass `--force` to overwrite.

## Excel schema

The sheet must have a header row. Columns:

| Column     | Required | Example                  | Notes                                              |
|------------|----------|--------------------------|----------------------------------------------------|
| `text`     | yes      | `Battery is low.`        | Exactly what Piper will speak.                     |
| `filename` | yes      | `battery_low.wav`        | Output filename. Include `.wav`.                   |
| `folder`   | yes      | `phrases`                | Sub-path under `tts/`. Use `/` for nesting.     |
| `voice`    | no       | `en_GB-alan-medium`        | Blank = use `--default-voice` (`en_GB-alan-medium`).|

Blank rows are skipped. Voice names follow Piper's naming
(`<lang>-<speaker>-<quality>`).

### Duplicate removal

At the start of every run, the script de-duplicates `wordlist.xlsx` **in
place**. A duplicate is any row whose `folder` + `filename` pair (its output
WAV path) matches an earlier row; the first occurrence is kept and later
ones are deleted from the file. Each removal is printed.

Note: the same `filename` under *different* folders is not a duplicate -
`words/common/no.wav` and `words/determiners/no.wav` are distinct outputs.

### Silence rows

For inserting micro-gaps between concatenated words, set `text` to the
sentinel `<silence:N>` where `N` is milliseconds. The script writes pure
zero samples directly (Piper is not invoked). Example:

| text             | filename          | folder    | voice |
|------------------|-------------------|-----------|-------|
| `<silence:100>`  | `pause_100ms.wav` | `silence` |       |

## Voices

- Default: **`en_GB-alan-medium`** - solid quality, fast, ~60 MB model.
- For a higher-fidelity UK English comparison: **`en_GB-cori-high`** (alan
  itself only ships in `low` and `medium`).

Put the voice name in the `voice` column to override per row. Piper will
download each model the first time it's used. Browse all available voices
at <https://huggingface.co/rhasspy/piper-voices/tree/main> — names follow
`<lang>-<speaker>-<quality>`.

## manifest.json

Every run writes `tts/manifest.json` alongside the WAVs. It is the index
the device loads into RAM at boot to drive the phrase/word lookup tiers.

Schema:

```json
{
  "generated_at": "2026-05-16T22:42:52Z",
  "format": { "sample_rate": 22050, "bits": 16, "channels": 1 },
  "phrases": [
    { "text": "battery is low", "file": "/tts/phrases/battery_low.wav" }
  ],
  "words": [
    { "text": "zero", "file": "/tts/words/numbers/0.wav", "category": "numbers" }
  ]
}
```

Rules:

- `text` is normalised: lowercased, punctuation stripped, whitespace
  collapsed. The on-device matcher **must apply the same rule** to incoming
  text before lookup, or matches will silently miss.
- Phrase tier: every row with `folder == "phrases"`.
- Word tier: every row with `folder` starting `words/`. The path tail
  becomes `category` (`numbers`, `units`, `nouns`, …).
- `file` is the on-SD absolute path, not the local `tts/` path.
- Other folders (e.g. `silence/`) are not in the lookup tiers; the device
  addresses them directly by path.

Rebuild the manifest without re-synthesising:

```bash
python generate_waves.py --manifest-only
```

## Cleaning up before git

The generated `tts/` library (WAVs + `manifest.json`) and the downloaded
`voices/` models are both regenerable - the library from `wordlist.xlsx`, the
voices by re-download on the next run.
These are gitignored, so neither should be committed, but clean is provided to 
delete both in one step for other sharing:

```bash
python generate_waves.py --clean
```

## Limitations / Improvements

### Word-bank prosody: no carrier-phrase cropping (yet)

In `Recording_Guidelines.txt` we warn that single-word recordings made with
sentence-final falling tone sound wrong when concatenated mid-sentence. A
mitigation is to wrap each word-bank entry in a carrier phrase ("say X
again"), synthesise, then crop the middle word out.

**We are not doing that in this version.**:

- It adds non-trivial complexity (forced alignment or per-row crop markers)
  and the problem actually isn't too bad with Piper's neural prosody.
- Piper's per-word output is noticeably less sing-songy than classic
  concatenative or formant TTS, so the original guideline (written with
  human recordings in mind) may not bite as hard.

## Files

```
TTS_WaveGen/
  generate_waves.py    Main script
  wordlist.xlsx        Word/phrase list (created by --init, edit freely)
  requirements.txt     Python deps
  README.md            This file
  voices/              Piper models, downloaded on demand (gitignored)
  tts/              Generated WAVs (gitignored)
```
