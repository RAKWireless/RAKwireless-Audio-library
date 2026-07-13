#!/usr/bin/env bash
# Flash Espressif TTS voice .dat into voice_data partition (offset 0x400000).
# ESP32-S3 / RAK3112: use --voice xiaole (default).
# Usage: ./flash_voice.sh --port /dev/tty.usbserial-XXXX [--voice xiaole]
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT=""
VOICE="xiaole"

usage() {
    echo "Usage: $0 --port <serial-port> [--voice xiaole|xiaoxin_small|xiaoxin|xiaoxin_custom]" >&2
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --port|-p)
            PORT="$2"; shift 2 ;;
        --voice|-v)
            VOICE="$2"; shift 2 ;;
        -h|--help)
            usage ;;
        *)
            echo "Unknown argument: $1" >&2
            usage ;;
    esac
done

[[ -z "$PORT" ]] && usage

case "$VOICE" in
    xiaole|xiaoxin_small|xiaoxin|xiaoxin_custom) ;;
    *)
        echo "ERROR: --voice must be one of xiaole, xiaoxin_small, xiaoxin, xiaoxin_custom" >&2
        exit 1 ;;
esac

DAT_NAME="esp_tts_voice_data_${VOICE}.dat"
DAT_OUT="$ROOT/data/$DAT_NAME"
OFFSET="0x400000"
PARTITION_BYTES=$((0x300000))

case "$VOICE" in
    xiaole)         VOICE_INFO="ESP32-S3 recommended: xiaole female voice (~2.8 MB)" ;;
    xiaoxin_small)  VOICE_INFO="xiaoxin compact, robotic (~2.8 MB); needs TTS_VOICE_XIAOXIN in build_opt.h" ;;
    xiaoxin)        VOICE_INFO="xiaoxin standard (~3.8 MB); partition too small by default" ;;
    xiaoxin_custom) VOICE_INFO="xiaoxin custom (~3.7 MB); partition too small by default" ;;
esac

ESP_SR="$ROOT/esp-sr"
DAT_SRC="$ESP_SR/esp-tts/esp_tts_chinese/$DAT_NAME"
DAT_URL="https://github.com/espressif/esp-sr/raw/master/esp-tts/esp_tts_chinese/$DAT_NAME"

ensure_voice_file() {
    if [[ -f "$DAT_SRC" ]]; then
        return
    fi
    if [[ ! -d "$ESP_SR" ]]; then
        echo "Cloning esp-sr (voice data)..."
        git clone --depth 1 --filter=blob:none --sparse https://github.com/espressif/esp-sr.git "$ESP_SR"
        (cd "$ESP_SR" && git sparse-checkout set esp-tts/esp_tts_chinese)
    fi
    if [[ ! -f "$DAT_SRC" ]]; then
        echo "Downloading $DAT_URL ..."
        mkdir -p "$(dirname "$DAT_OUT")"
        curl -L --connect-timeout 60 -o "$DAT_OUT" "$DAT_URL"
        if [[ ! -f "$DAT_OUT" ]] || [[ $(stat -f%z "$DAT_OUT") -lt 100000 ]]; then
            echo "ERROR: download failed: $DAT_OUT"
            exit 1
        fi
        return
    fi
}

echo "Voice: $VOICE - $VOICE_INFO"
if [[ "$VOICE" != "xiaole" ]]; then
    echo "NOTE: ESP32-S3 Arduino 3.x links voice_set_xiaole only."
    echo "      For xiaoxin voices add -DTTS_VOICE_XIAOXIN to build_opt.h and rebuild."
fi

ensure_voice_file

mkdir -p "$(dirname "$DAT_OUT")"
if [[ -f "$DAT_SRC" ]]; then
    cp -f "$DAT_SRC" "$DAT_OUT"
fi

if [[ ! -f "$DAT_OUT" ]]; then
    echo "ERROR: voice file not found: $DAT_OUT"
    exit 1
fi

SIZE=$(stat -f%z "$DAT_OUT")
if [[ "$SIZE" -gt "$PARTITION_BYTES" ]]; then
    echo "ERROR: $DAT_NAME is $SIZE bytes but voice_data partition is $PARTITION_BYTES bytes."
    echo "       Enlarge voice_data in partitions.csv (e.g. 0x3C0000) and reflash firmware first."
    exit 1
fi

ESPTOOL=$(find "$HOME/Library/Arduino15/packages/esp32/tools/esptool_py" -maxdepth 2 -type f -name "esptool" 2>/dev/null | sort -t/ -k9 -V | tail -1)

if [[ -z "$ESPTOOL" ]]; then
    echo "ERROR: esptool not found. Install ESP32 board package in Arduino IDE."
    exit 1
fi

echo "Using $ESPTOOL"
echo "Flashing $DAT_OUT ($SIZE bytes) -> $OFFSET on $PORT"
"$ESPTOOL" --chip esp32s3 --port "$PORT" --baud 921600 write_flash "$OFFSET" "$DAT_OUT"
echo "Done. Reboot board, then send STATUS and TTS command on serial."
