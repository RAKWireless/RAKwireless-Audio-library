/**
 * @file AudioSmokeTest_RAK18003.ino
 * @brief Smoke test for the RAK3112 audio path: speaker, then microphone.
 *
 * Repeats:
 *   1. Play a 1 kHz sine tone for TONE_MS at SAMPLE_RATE through the
 *      TAS2560 amplifier(s) on I2S.
 *   2. Mute the amps, stop I2S, start the PDM mic and record REC_MS.
 *   3. Print min / max / avg / RMS of the recording.
 *
 * Listen for a clean tone on the speaker(s) and silence while the mic
 * records. The two legs run one after the other; for mic and speaker
 * at the same time see PDMSerialPlotter_RAK18003 (FULL_DUPLEX_TEST).
 *
 * Hardware: RAK3112 core + RAK19007 base + RAK18003 interposer +
 *           RAK18030 PDM mic + RAK18060 (stereo) or RAK18061 (mono) amp.
 *
 * Set SAMPLE_RATE to the rate under test (8000 / 16000 / 22050 / 48000).
 * Set AMP_VARIANT to 60 for RAK18060 (stereo, two TAS2560s) or 61 for
 * RAK18061 (mono, one TAS2560).
 */

#include "audio.h"
#include <math.h>

// ---- knobs --------------------------------------------------------------
#define SAMPLE_RATE     16000   // 8000 / 16000 / 22050 / 48000
#define AMP_VARIANT     60      // 60 = RAK18060 stereo, 61 = RAK18061 mono
#define TONE_HZ         1000
#define TONE_MS         2000
#define REC_MS          2000
#define AMP_GAIN        1       // TAS2560 set_gain 0..15
// ------------------------------------------------------------------------

#define AMP_LEFT_ADDRESS    0x4c
#define AMP_RIGHT_ADDRESS   0x4f

TPT29555 Expander1(0x23);
TPT29555 Expander2(0x25);

TAS2560 AMP_Left;
#if AMP_VARIANT == 60
TAS2560 AMP_Right;
#endif

static const int channels = 1;

// Tone playback buffer — one period of 1 kHz @ SAMPLE_RATE, repeated.
static const int TONE_SAMPLES_PER_BLOCK = SAMPLE_RATE / 10;  // 100 ms
static int16_t toneBlock[TONE_SAMPLES_PER_BLOCK];

// Mic capture buffer (16-bit mono). Capped to keep static DRAM use
// under ~64 KB so the high sample rates (88 200 / 96 000) link on
// ESP32-S3 — a full REC_MS allocation at 96 kHz would overflow
// dram0_0_seg. The effective recording window at high rates is
// shorter than REC_MS; fine for a smoke-test min/max/RMS readout.
static const int REC_BUF_SAMPLES_CAP = 32000;          // 64 KB
static const int REC_SAMPLES_REQUESTED = (SAMPLE_RATE * REC_MS) / 1000;
static const int REC_SAMPLES =
    (REC_SAMPLES_REQUESTED < REC_BUF_SAMPLES_CAP) ? REC_SAMPLES_REQUESTED
                                                  : REC_BUF_SAMPLES_CAP;
static int16_t recBuf[REC_SAMPLES];

static void buildToneBlock()
{
  const float w = 2.0f * 3.14159265358979f * (float)TONE_HZ / (float)SAMPLE_RATE;
  for (int i = 0; i < TONE_SAMPLES_PER_BLOCK; i++) {
    toneBlock[i] = (int16_t)(0.3f * 32767.0f * sinf(w * i));
  }
}

static void rak18003Init()
{
  while (!Expander1.begin()) {
    Serial.println("Waiting for RAK18003 expander 1...");
    delay(200);
  }
  while (!Expander2.begin()) {
    Serial.println("Waiting for RAK18003 expander 2...");
    delay(200);
  }
  for (int p = 0; p < 16; p++) Expander1.pinMode(p, INPUT);
  for (int p = 0; p < 16; p++) Expander2.pinMode(p, INPUT);
  Expander2.pinMode(0, OUTPUT); Expander2.digitalWrite(0, 1);  // CS1 high
  Expander2.pinMode(1, OUTPUT); Expander2.digitalWrite(1, 1);  // CS2 high
  Expander2.pinMode(2, OUTPUT); Expander2.digitalWrite(2, 1);  // CS3 high
  Expander2.pinMode(3, OUTPUT); Expander2.digitalWrite(3, 0);  // PDM → core
}

static void dumpTas2560Status(const char *label, TAS2560 &amp)
{
  Serial.printf("[%s] PWR=0x%02X INT1=0x%02X INT2=0x%02X\r\n",
                label,
                amp.get_power_status(),
                amp.read_INT_DET1(),
                amp.read_INT_DET2());
}

static void playTone()
{
  Serial.println("--- speaker leg ---");

  // Pass SAMPLE_RATE so the amp loads the register table, PCM_RATE and
  // PLL settings for that rate. Without it the chip is set up for
  // 16 kHz and plays nothing at 8 kHz.
  if (!AMP_Left.begin(AMP_LEFT_ADDRESS, (uint32_t)SAMPLE_RATE)) {
    Serial.println("AMP_Left.begin failed");
    return;
  }
  AMP_Left.set_pcm_channel(LeftMode);
  AMP_Left.set_gain(AMP_GAIN);

#if AMP_VARIANT == 60
  if (!AMP_Right.begin(AMP_RIGHT_ADDRESS, (uint32_t)SAMPLE_RATE)) {
    Serial.println("AMP_Right.begin failed");
  } else {
    AMP_Right.set_pcm_channel(RightMode);
    AMP_Right.set_gain(AMP_GAIN);
  }
#endif

  I2S.setPins(WB_IO6, WB_IO5, SDA1, SCL1);
  if (!I2S.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT,
                 (i2s_slot_mode_t)channels)) {
    Serial.println("I2S.begin failed");
    return;
  }

  buildToneBlock();

  const uint32_t end = millis() + TONE_MS;
  uint32_t blocks = 0;
  while ((int32_t)(end - millis()) > 0) {
    I2S.write((const uint8_t *)toneBlock, sizeof(toneBlock));
    blocks++;
  }
  Serial.printf("Wrote %u tone blocks (%d samples each)\r\n",
                blocks, TONE_SAMPLES_PER_BLOCK);

  dumpTas2560Status("after-play L", AMP_Left);
#if AMP_VARIANT == 60
  dumpTas2560Status("after-play R", AMP_Right);
#endif

  // Mute the amps BEFORE stopping I²S. If the clocks stop with the
  // amp powered, the class-D idles and hisses or freewheels a stale
  // tone. set_mute() writes PWR_CTRL_1 = 0x03.
  AMP_Left.set_mute();
#if AMP_VARIANT == 60
  AMP_Right.set_mute();
#endif

  I2S.end();
}

static void recordMic()
{
  Serial.println("--- mic leg ---");

  if (!PDM.begin(channels, SAMPLE_RATE)) {
    Serial.println("PDM.begin failed");
    return;
  }

  int written = 0;
  const int wantBytes = REC_SAMPLES * sizeof(int16_t);
  const uint32_t deadline = millis() + REC_MS + 500;

  while (written < wantBytes && (int32_t)(deadline - millis()) > 0) {
    // PDM.read returns what's ready, up to the requested size, and 0
    // if nothing is ready yet.
    int chunk = wantBytes - written;
    int got = PDM.read(((uint8_t *)recBuf) + written, chunk);
    if (got <= 0) { delay(1); continue; }
    written += got;
  }

  PDM.end();

  const int gotSamples = written / sizeof(int16_t);
  if (gotSamples == 0) {
    Serial.println("No samples captured");
    return;
  }

  int32_t mn = INT32_MAX, mx = INT32_MIN;
  int64_t sum = 0;
  uint64_t sqsum = 0;
  for (int i = 0; i < gotSamples; i++) {
    int32_t s = recBuf[i];
    if (s < mn) mn = s;
    if (s > mx) mx = s;
    sum += s;
    sqsum += (uint64_t)((int64_t)s * s);
  }
  const float avg = (float)sum / gotSamples;
  const float rms = sqrtf((float)sqsum / gotSamples);

  Serial.printf("samples=%d  min=%ld  max=%ld  avg=%.1f  rms=%.1f\r\n",
                gotSamples, (long)mn, (long)mx, avg, rms);
}

void setup()
{
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);

  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(50);

  Serial.println();
  Serial.println("=== RAK3112 Audio Smoke Test ===");
  Serial.printf("SAMPLE_RATE=%d  AMP_VARIANT=RAK180%d  channels=%d\r\n",
                SAMPLE_RATE, AMP_VARIANT, channels);

  rak18003Init();
}

void loop()
{
  playTone();
  delay(200);
  recordMic();
  Serial.println("--- pass complete; sleeping 5 s ---");
  delay(5000);
}
