#include "esp_tts_bridge.h"

#include <stdlib.h>
#include <string.h>

static const int kEspTtsRate = 16000;
static const int kPipelineRate = 22050;

bool espTtsContainsChinese(const String& text)
{
  const uint8_t* p = (const uint8_t*)text.c_str();
  while (*p)
  {
    if (*p < 0x80) { p++; continue; }
    if (*p >= 0xE4 && *p <= 0xE9 && p[1] >= 0x80 && p[2] >= 0x80)
      return true;
    if ((*p & 0xE0) == 0xE0 && p[1] && p[2])
    {
      uint32_t cp = ((uint32_t)(p[0] & 0x0F) << 12)
                  | ((uint32_t)(p[1] & 0x3F) << 6)
                  |  (uint32_t)(p[2] & 0x3F);
      if (cp >= 0x4E00 && cp <= 0x9FFF) return true;
    }
    p++;
  }
  return false;
}

#if defined(ENABLE_ESPRESSIF_TTS) || defined(TTS_ENABLE_ESPRESSIF)

extern "C" {
#include "esp_tts.h"
#include "esp_partition.h"
#if defined(TTS_VOICE_XIAOXIN)
#include "esp_tts_voice_template.h"
#else
#include "esp_tts_voice_xiaole.h"
#endif
}

static esp_tts_handle_t g_tts = nullptr;
static esp_tts_voice_t* g_voice = nullptr;
static esp_partition_mmap_handle_t g_mmap = 0;
static bool g_ready = false;

static bool resamplePcm(const int16_t* in, size_t inSamples, int inRate,
                        int outRate, int16_t** out, size_t* outSamples)
{
  if (!in || inSamples == 0 || !out || !outSamples || inRate <= 0 || outRate <= 0)
    return false;
  if (inRate == outRate)
  {
    int16_t* buf = (int16_t*)ps_malloc(inSamples * sizeof(int16_t));
    if (!buf) buf = (int16_t*)malloc(inSamples * sizeof(int16_t));
    if (!buf) return false;
    memcpy(buf, in, inSamples * sizeof(int16_t));
    *out = buf;
    *outSamples = inSamples;
    return true;
  }

  const double ratio = (double)outRate / (double)inRate;
  size_t nOut = (size_t)((double)inSamples * ratio + 0.5);
  if (nOut == 0) return false;

  int16_t* buf = (int16_t*)ps_malloc(nOut * sizeof(int16_t));
  if (!buf) buf = (int16_t*)malloc(nOut * sizeof(int16_t));
  if (!buf) return false;

  for (size_t i = 0; i < nOut; i++)
  {
    double src = (double)i / ratio;
    size_t idx = (size_t)src;
    double frac = src - (double)idx;
    int16_t a = in[idx < inSamples ? idx : (inSamples - 1)];
    int16_t b = in[(idx + 1 < inSamples) ? (idx + 1) : (inSamples - 1)];
    double v = (double)a * (1.0 - frac) + (double)b * frac;
    if (v > 32767.0) v = 32767.0;
    if (v < -32768.0) v = -32768.0;
    buf[i] = (int16_t)(v + (v >= 0 ? 0.5 : -0.5));
  }
  *out = buf;
  *outSamples = nOut;
  return true;
}

// Trim leading silence from a stream chunk (reduces double-padding between chunks).
static size_t skipLeadingSilence(const int16_t* s, size_t n)
{
  size_t i = 0;
  while (i < n && s[i] > -80 && s[i] < 80) i++;
  return (i < n) ? i : 0;
}

bool espTtsSetup()
{
  if (g_ready) return true;

  const esp_partition_t* part = esp_partition_find_first(
      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, "voice_data");
  if (!part)
  {
    Serial.println("Espressif TTS: voice_data partition not found");
    return false;
  }

  const void* voicedata = nullptr;
  esp_err_t err = esp_partition_mmap(
      part, 0, part->size, ESP_PARTITION_MMAP_DATA, &voicedata, &g_mmap);
  if (err != ESP_OK || !voicedata)
  {
    Serial.printf("Espressif TTS: mmap voice_data failed (%d)\n", (int)err);
    return false;
  }

#if defined(TTS_VOICE_XIAOXIN)
  g_voice = esp_tts_voice_set_init(&esp_tts_voice_template, (void*)voicedata);
#else
  g_voice = esp_tts_voice_set_init(&esp_tts_voice_xiaole, (void*)voicedata);
#endif
  if (!g_voice)
  {
    Serial.println("Espressif TTS: voice_set_init failed (wrong .dat for this build?)");
    Serial.println("  ESP32-S3: flash esp_tts_voice_data_xiaole.dat");
    return false;
  }

  g_tts = esp_tts_create(g_voice);
  if (!g_tts)
  {
    Serial.println("Espressif TTS: esp_tts_create failed");
    esp_tts_voice_set_free(g_voice);
    g_voice = nullptr;
    return false;
  }

  g_ready = true;
#if defined(TTS_VOICE_XIAOXIN)
  Serial.printf("Espressif TTS ready (xiaoxin): %d Hz -> %d Hz\n", kEspTtsRate, kPipelineRate);
#else
  Serial.printf("Espressif TTS ready (xiaole): %d Hz -> %d Hz\n", kEspTtsRate, kPipelineRate);
#endif
  return true;
}

bool espTtsIsReady() { return g_ready; }
int  espTtsNativeSampleRate() { return kEspTtsRate; }

bool espTtsRenderChinese(const String& text, uint8_t** outPcm, size_t* outLen,
                         int targetSampleRate)
{
  if (!g_ready || !g_tts || !outPcm || !outLen) return false;
  *outPcm = nullptr;
  *outLen = 0;

  if (!espTtsContainsChinese(text))
  {
    Serial.println("Espressif TTS: text has no Chinese UTF-8");
    return false;
  }

  if (targetSampleRate <= 0) targetSampleRate = kPipelineRate;

  esp_tts_stream_reset(g_tts);
  if (!esp_tts_parse_chinese(g_tts, text.c_str()))
  {
    Serial.printf("Espressif TTS: parse failed: %s\n", text.c_str());
    return false;
  }

  size_t cap = 64 * 1024;
  size_t len = 0;
  int16_t* acc = (int16_t*)ps_malloc(cap);
  if (!acc) acc = (int16_t*)malloc(cap);
  if (!acc) return false;

  bool firstChunk = true;
  for (;;)
  {
    int chunkSamples = 0;
    short* chunk = esp_tts_stream_play(g_tts, &chunkSamples, 3);
    if (chunkSamples <= 0) break;

    const int16_t* src = (const int16_t*)chunk;
    size_t cnt = (size_t)chunkSamples;
    if (!firstChunk)
    {
      size_t skip = skipLeadingSilence(src, cnt);
      if (skip >= cnt) continue;
      src += skip;
      cnt -= skip;
    }
    firstChunk = false;

    size_t need = len + cnt;
    if (need * sizeof(int16_t) > cap)
    {
      size_t newCap = cap;
      while (newCap < need * sizeof(int16_t)) newCap *= 2;
      int16_t* bigger = (int16_t*)ps_malloc(newCap);
      if (!bigger) bigger = (int16_t*)realloc(acc, newCap);
      else { memcpy(bigger, acc, len * sizeof(int16_t)); free(acc); }
      if (!bigger) { free(acc); return false; }
      acc = bigger;
      cap = newCap;
    }
    memcpy(acc + len, src, cnt * sizeof(int16_t));
    len += cnt;
  }

  if (len == 0) { free(acc); return false; }

  int16_t* outBuf = nullptr;
  size_t outSamples = 0;
  if (!resamplePcm(acc, len, kEspTtsRate, targetSampleRate, &outBuf, &outSamples))
  {
    free(acc);
    return false;
  }
  free(acc);

  *outLen = outSamples * sizeof(int16_t);
  *outPcm = (uint8_t*)outBuf;
  return true;
}

void espTtsShutdown()
{
  if (g_tts) { esp_tts_destroy(g_tts); g_tts = nullptr; }
  if (g_voice) { esp_tts_voice_set_free(g_voice); g_voice = nullptr; }
  if (g_mmap) { esp_partition_munmap(g_mmap); g_mmap = 0; }
  g_ready = false;
}

#else  // !ENABLE_ESPRESSIF_TTS

bool espTtsSetup() { return false; }
bool espTtsIsReady() { return false; }
int  espTtsNativeSampleRate() { return kEspTtsRate; }

bool espTtsRenderChinese(const String&, uint8_t**, size_t*, int)
{
  return false;
}

void espTtsShutdown() {}

#endif
