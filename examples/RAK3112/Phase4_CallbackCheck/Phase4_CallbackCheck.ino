/**
 * @file Phase4_CallbackCheck.ino
 * @brief Verifies the Phase 4 PDM onReceive() plumbing against the
 *        Checkpoint 4 bullets in driver_updates_plan.md.
 *
 * Sub-tests:
 *  1) Sustained-capture: 30 s with a callback registered. Reports
 *     DMA events, dropped events, user-callback invocations, and
 *     the driver-owned dispatch latency (ISR → user cb entry) as
 *     rolling min/mean/max.
 *  2) Callback replacement: swap the callback pointer mid-run; the
 *     new callback should start firing and the old one should stop.
 *  3) Detach + teardown: pass nullptr to onReceive(), then call
 *     end(). No callback should fire after detach.
 *
 * At 16 kHz mono 16-bit, expected DMA period is
 *   dma_frame_num / rate = 240 / 16000 = 15 ms
 * so ~2000 events over 30 s. Driver-owned dispatch latency should
 * be comfortably under one DMA period.
 *
 * Hardware: RAK3112 core + RAK19007 base + RAK18003 interposer +
 *           RAK18030 PDM mic.
 */

#include "audio.h"

#define SAMPLE_RATE  16000
#define CHANNELS     1

TPT29555 Expander1(0x23);
TPT29555 Expander2(0x25);

static uint8_t g_scratch[512];

// Callback bookkeeping — the driver counters cover the ISR/dispatch
// path, but we also want to measure user-callback-body characteristics.
volatile unsigned long cb_body_time_min_us = 0xFFFFFFFF;
volatile unsigned long cb_body_time_max_us = 0;
volatile unsigned long cb_body_time_sum_us = 0;

volatile unsigned long dispatch_lat_min_us = 0xFFFFFFFF;
volatile unsigned long dispatch_lat_max_us = 0;
volatile unsigned long dispatch_lat_sum_us = 0;
volatile unsigned long dispatch_lat_count  = 0;

volatile unsigned long cb_A_count = 0;
volatile unsigned long cb_B_count = 0;

static void cb_consume_A()
{
  // Measure dispatch latency: micros() at entry minus the ISR timestamp.
  unsigned long isr_ts = PDM.lastIsrMicros();
  unsigned long entry  = (unsigned long)esp_timer_get_time();
  if (isr_ts != 0 && entry > isr_ts) {
    unsigned long lat = entry - isr_ts;
    if (lat < dispatch_lat_min_us) dispatch_lat_min_us = lat;
    if (lat > dispatch_lat_max_us) dispatch_lat_max_us = lat;
    dispatch_lat_sum_us += lat;
    dispatch_lat_count++;
  }

  // Actual user work: drain staging.
  int drained_total = 0;
  while (PDM.available() > 0) {
    int got = PDM.read(g_scratch, sizeof(g_scratch));
    if (got <= 0) break;
    drained_total += got;
    if (drained_total >= 4096) break;  // cap per invocation
  }

  unsigned long body = (unsigned long)esp_timer_get_time() - entry;
  if (body < cb_body_time_min_us) cb_body_time_min_us = body;
  if (body > cb_body_time_max_us) cb_body_time_max_us = body;
  cb_body_time_sum_us += body;

  cb_A_count++;
}

static void cb_consume_B()
{
  // Simpler variant used for replacement test.
  while (PDM.available() > 0) {
    if (PDM.read(g_scratch, sizeof(g_scratch)) <= 0) break;
  }
  cb_B_count++;
}

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

static void reset_stats()
{
  cb_body_time_min_us = 0xFFFFFFFF;
  cb_body_time_max_us = 0;
  cb_body_time_sum_us = 0;
  dispatch_lat_min_us = 0xFFFFFFFF;
  dispatch_lat_max_us = 0;
  dispatch_lat_sum_us = 0;
  dispatch_lat_count  = 0;
  cb_A_count = 0;
  cb_B_count = 0;
}

static void test1_sustained()
{
  Serial.println();
  Serial.println("=== Test 1: sustained 30 s capture via callback ===");
  Serial.printf("Expected DMA period @ %d Hz mono 16-bit: ~15 ms\r\n", SAMPLE_RATE);
  Serial.println("(240 frames / 16000 Hz). Expected count: ~2000 in 30 s.");

  reset_stats();
  unsigned long dma0 = PDM.dmaEvents();
  unsigned long drop0 = PDM.droppedEvents();
  unsigned long cb0 = PDM.userCallbacks();

  PDM.onReceive(cb_consume_A);

  const uint32_t start = millis();
  while (millis() - start < 30000) {
    delay(3000);
    Serial.printf("  t=%lus  dma=%lu  drop=%lu  driverCB=%lu  A=%lu\r\n",
                  (unsigned long)((millis()-start)/1000),
                  PDM.dmaEvents() - dma0,
                  PDM.droppedEvents() - drop0,
                  PDM.userCallbacks() - cb0,
                  cb_A_count);
  }

  unsigned long dma_total = PDM.dmaEvents() - dma0;
  unsigned long drop_total = PDM.droppedEvents() - drop0;
  unsigned long cb_total = PDM.userCallbacks() - cb0;

  Serial.println();
  Serial.printf("DMA events (30 s):       %lu\r\n", dma_total);
  Serial.printf("Dropped events:          %lu\r\n", drop_total);
  Serial.printf("Driver user-cb count:    %lu\r\n", cb_total);
  Serial.printf("User cb_A_count:         %lu\r\n", cb_A_count);
  if (cb_A_count > 0) {
    Serial.printf("cb body  us  min/mean/max: %lu / %lu / %lu\r\n",
                  cb_body_time_min_us,
                  cb_body_time_sum_us / cb_A_count,
                  cb_body_time_max_us);
  }
  if (dispatch_lat_count > 0) {
    Serial.printf("dispatch us  min/mean/max: %lu / %lu / %lu\r\n",
                  dispatch_lat_min_us,
                  dispatch_lat_sum_us / dispatch_lat_count,
                  dispatch_lat_max_us);
    Serial.printf("(samples with valid ISR ts: %lu)\r\n", dispatch_lat_count);
  }
}

static void test2_replacement()
{
  Serial.println();
  Serial.println("=== Test 2: callback replacement ===");
  // Snapshot before swap.
  unsigned long A_before = cb_A_count;
  unsigned long B_before = cb_B_count;
  PDM.onReceive(cb_consume_B);
  delay(1000);
  unsigned long A_after = cb_A_count;
  unsigned long B_after = cb_B_count;

  Serial.printf("during 1 s post-swap: A delta = %lu, B delta = %lu\r\n",
                A_after - A_before, B_after - B_before);
  bool pass = (A_after == A_before) && (B_after > B_before);
  Serial.printf("A stopped and B started: %s\r\n", pass ? "PASS" : "FAIL");
}

static void test3_detach()
{
  Serial.println();
  Serial.println("=== Test 3: detach + no callbacks after end() ===");
  PDM.onReceive(nullptr);
  unsigned long A_before = cb_A_count;
  unsigned long B_before = cb_B_count;
  delay(500);
  unsigned long A_after = cb_A_count;
  unsigned long B_after = cb_B_count;
  Serial.printf("during 500 ms detached: A delta = %lu, B delta = %lu\r\n",
                A_after - A_before, B_after - B_before);
  bool pass = (A_after == A_before) && (B_after == B_before);
  Serial.printf("no callbacks after detach: %s\r\n", pass ? "PASS" : "FAIL");

  PDM.end();
  Serial.println("PDM.end() returned cleanly.");
}

void setup()
{
  pinMode(WB_IO2, OUTPUT); digitalWrite(WB_IO2, HIGH);
  pinMode(LED_GREEN, OUTPUT); digitalWrite(LED_GREEN, HIGH);
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(50);

  Serial.println();
  Serial.println("=== Phase 4 PDM callback check ===");
  Serial.printf("SAMPLE_RATE=%d CHANNELS=%d\r\n", SAMPLE_RATE, CHANNELS);

  rak18003Init();
  if (!PDM.begin(CHANNELS, SAMPLE_RATE)) {
    Serial.println("PDM.begin failed");
    while (1) yield();
  }

  test1_sustained();
  test2_replacement();
  test3_detach();

  Serial.println();
  Serial.println("=== Phase 4 checks complete ===");
}

void loop()
{
  delay(1000);
}
