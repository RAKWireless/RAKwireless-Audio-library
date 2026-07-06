/**
 * @file Phase5_APIParityCheck.ino
 * @brief Verifies the Phase 5 buffer-sizing / gain API against the
 *        Checkpoint 5 bullets in driver_updates_plan.md.
 *
 * Sub-tests:
 *  1) setBufferSize() before begin() takes effect. Reports
 *     PDM.bufferSize() before and after, and callback rate over
 *     3 s for both a "small" (256 B) and "default" (~1920 B)
 *     configuration. Small → higher callback rate.
 *  2) setBufferSize() after begin() returns false and leaves the
 *     configured size unchanged.
 *  3) setGain() changes measured PDM RMS. Sample RMS at low gain
 *     (0), mid (40), and high (80); RMS should climb monotonically.
 *
 * Hardware: RAK3112 core + RAK19007 base + RAK18003 interposer +
 *           RAK18030 PDM mic. Speaker not required.
 */

#include "audio.h"
#include <math.h>

#define SAMPLE_RATE  16000
#define CHANNELS     1

TPT29555 Expander1(0x23);
TPT29555 Expander2(0x25);

static uint8_t g_scratch[512];
static volatile unsigned long cb_count = 0;

static void rak18003Init()
{
  while (!Expander1.begin()) { delay(200); }
  while (!Expander2.begin()) { delay(200); }
  for (int p = 0; p < 16; p++) Expander1.pinMode(p, INPUT);
  for (int p = 0; p < 16; p++) Expander2.pinMode(p, INPUT);
  Expander2.pinMode(0, OUTPUT); Expander2.digitalWrite(0, 1);
  Expander2.pinMode(1, OUTPUT); Expander2.digitalWrite(1, 1);
  Expander2.pinMode(2, OUTPUT); Expander2.digitalWrite(2, 1);
  Expander2.pinMode(3, OUTPUT); Expander2.digitalWrite(3, 0);
}

static void cb_drain()
{
  while (PDM.available() > 0) {
    if (PDM.read(g_scratch, sizeof(g_scratch)) <= 0) break;
  }
  cb_count++;
}

// Sample RMS across ~200 ms of PDM data.
static float measure_rms_over_200ms()
{
  int16_t buf[512];
  const uint32_t deadline = millis() + 200;
  int64_t sqsum = 0;
  unsigned long samples = 0;
  while (millis() < deadline) {
    int got = PDM.read(buf, sizeof(buf));
    if (got <= 0) { delay(1); continue; }
    int n = got / 2;
    for (int i = 0; i < n; i++) {
      int32_t s = buf[i];
      sqsum += (int64_t)s * s;
    }
    samples += n;
  }
  if (samples == 0) return 0.0f;
  return sqrtf((float)sqsum / samples);
}

static void test1_buffer_size_before_begin()
{
  Serial.println();
  Serial.println("=== Test 1: setBufferSize() before begin() ===");

  int rates[2];
  const int sizes[2] = { 256, 1920 };
  const char *labels[2] = { "small(256)", "default(1920)" };

  for (int i = 0; i < 2; i++) {
    PDM.end();  // safe if not open
    bool ok = PDM.setBufferSize(sizes[i]);
    Serial.printf("  setBufferSize(%d) returned %s\r\n",
                  sizes[i], ok ? "true" : "false");
    if (!PDM.begin(CHANNELS, SAMPLE_RATE)) {
      Serial.println("  PDM.begin failed");
      return;
    }
    int effective = PDM.bufferSize();
    Serial.printf("  effective bufferSize: %d bytes\r\n", effective);

    cb_count = 0;
    PDM.onReceive(cb_drain);
    unsigned long start = millis();
    delay(3000);
    unsigned long cb = cb_count;
    PDM.onReceive(nullptr);
    Serial.printf("  callbacks in 3 s: %lu (rate %.1f Hz)\r\n",
                  cb, cb / 3.0f);
    rates[i] = (int)cb;
    PDM.end();
  }

  Serial.printf("small vs default cb rate: %d vs %d — %s\r\n",
                rates[0], rates[1],
                rates[0] > rates[1] * 2 ? "PASS (small ≫ default)"
                                        : "check (small should be > default)");
}

static void test2_buffer_size_after_begin()
{
  Serial.println();
  Serial.println("=== Test 2: setBufferSize() after begin() rejected ===");
  PDM.setBufferSize(1024);
  if (!PDM.begin(CHANNELS, SAMPLE_RATE)) {
    Serial.println("  PDM.begin failed");
    return;
  }
  int before = PDM.bufferSize();
  bool ret = PDM.setBufferSize(256);
  int after = PDM.bufferSize();
  Serial.printf("  before=%d  setBufferSize(256) returned %s  after=%d\r\n",
                before, ret ? "true" : "false", after);
  bool pass = (ret == false) && (before == after);
  Serial.printf("  result: %s\r\n", pass ? "PASS" : "FAIL");
  PDM.end();
}

static void test3_gain_changes_rms()
{
  Serial.println();
  Serial.println("=== Test 3: setGain() changes RMS ===");
  PDM.setBufferSize(1920);
  if (!PDM.begin(CHANNELS, SAMPLE_RATE)) {
    Serial.println("  PDM.begin failed");
    return;
  }

  const int gains[3] = { 0, 40, 80 };
  float rms[3];
  for (int i = 0; i < 3; i++) {
    PDM.setGain(gains[i]);
    delay(300);  // let staging refill with post-reconfig samples
    // Drain stale.
    while (PDM.available() > 0) PDM.read(g_scratch, sizeof(g_scratch));
    rms[i] = measure_rms_over_200ms();
    Serial.printf("  gain=%d  rms=%.1f\r\n", gains[i], rms[i]);
  }
  bool monotonic = (rms[0] < rms[1]) && (rms[1] < rms[2]);
  Serial.printf("  RMS monotonically climbs: %s\r\n",
                monotonic ? "PASS" : "check (needs ambient sound)");
  PDM.end();
}

void setup()
{
  pinMode(WB_IO2, OUTPUT); digitalWrite(WB_IO2, HIGH);
  pinMode(LED_GREEN, OUTPUT); digitalWrite(LED_GREEN, HIGH);
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(50);
  Serial.println();
  Serial.println("=== Phase 5 API parity check ===");
  Serial.printf("SAMPLE_RATE=%d CHANNELS=%d\r\n", SAMPLE_RATE, CHANNELS);
  rak18003Init();

  test1_buffer_size_before_begin();
  test2_buffer_size_after_begin();
  test3_gain_changes_rms();

  Serial.println();
  Serial.println("=== Phase 5 checks complete ===");
}

void loop() { delay(1000); }
