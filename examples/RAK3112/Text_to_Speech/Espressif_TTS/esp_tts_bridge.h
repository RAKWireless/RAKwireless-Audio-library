#pragma once

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

// Espressif ESP-TTS (Chinese only, 16 kHz mono). Requires:
//   - Arduino ESP32 3.x (ESP-SR headers)
//   - TTS_ENABLE_ESPRESSIF at compile time
//   - voice_data partition + matching .dat flashed @ 0x400000
// ESP32-S3 (RAK3112): default xiaole — flash esp_tts_voice_data_xiaole.dat
// Optional -DTTS_VOICE_XIAOXIN + xiaoxin_small.dat (ESP32 classic only)

bool     espTtsContainsChinese(const String& text);
bool     espTtsSetup();
bool     espTtsIsReady();
int      espTtsNativeSampleRate();
// targetSampleRate: native 16000 (no resample), or pipeline rate e.g. 22050.
bool     espTtsRenderChinese(const String& text, uint8_t** outPcm, size_t* outLen,
                               int targetSampleRate = 16000);
void     espTtsShutdown();
