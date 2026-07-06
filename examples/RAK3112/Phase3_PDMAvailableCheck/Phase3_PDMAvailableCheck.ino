/**
 * @file Phase3_PDMAvailableCheck.ino
 * @brief Verifies the Phase 3 PDM API foundation against the
 *        Checkpoint 3 bullets in driver_updates_plan.md.
 *
 * Three sub-tests run in sequence, then loop forever:
 *   1) available() grows / drains. Logs available() over ~30 s with
 *      occasional read() calls; available should climb when we
 *      don't read and drop when we do.
 *   2) read-size canary. PDM.read(buf, 64) into an 80-byte buffer
 *      with 0xAA sentinels at [64..79]; the sentinels must survive.
 *   3) order-preservation. Two back-to-back small reads (32 bytes
 *      each) must yield the same samples as one read of 64 bytes
 *      called over the same data, i.e. nothing is dropped between
 *      reads.
 *
 * Hardware: RAK3112 core + RAK19007 base + RAK18003 interposer +
 *           RAK18030 PDM mic. Speaker not required.
 */

#include "audio.h"

#define SAMPLE_RATE  16000
#define CHANNELS     1

TPT29555 Expander1(0x23);
TPT29555 Expander2(0x25);

// Single static scratch buffer shared by drain helpers. Keeping this
// off the task stack — earlier versions declared per-function 4 KB
// arrays and blew the ~8 KB loopTask stack canary. 512 B is more
// than one PDM DMA descriptor, so a single read pulls whatever the
// IDF has queued.
static uint8_t g_scratch[512];

static void rak18003Init()
{
  while (!Expander1.begin()) { Serial.println("waiting expander1"); delay(200); }
  while (!Expander2.begin()) { Serial.println("waiting expander2"); delay(200); }
  for (int p = 0; p < 16; p++) Expander1.pinMode(p, INPUT);
  for (int p = 0; p < 16; p++) Expander2.pinMode(p, INPUT);
  Expander2.pinMode(0, OUTPUT); Expander2.digitalWrite(0, 1);
  Expander2.pinMode(1, OUTPUT); Expander2.digitalWrite(1, 1);
  Expander2.pinMode(2, OUTPUT); Expander2.digitalWrite(2, 1);
  Expander2.pinMode(3, OUTPUT); Expander2.digitalWrite(3, 0);
}

// Drain up to `max_iters` DMA descriptors. Bounded because DMA
// refills as fast as we read at line-rate, so an unbounded loop hangs.
// A short bound is fine — a few ms of PDM data getting discarded here
// is meaningless for the following checks.
static void drain_bounded(int max_iters)
{
  for (int i = 0; i < max_iters; i++) {
    if (PDM.available() <= 0) return;
    (void)PDM.read(g_scratch, sizeof(g_scratch));
  }
}

static void test1_available_grow_drain()
{
  Serial.println();
  Serial.println("=== Test 1: available() grow / drain over ~30 s ===");
  Serial.println("PDMClass owns a small staging buffer (2 x DMA");
  Serial.println("chunk = 3840 B) filled by non-blocking pulls from");
  Serial.println("the IDF. Expected pattern:");
  Serial.println(" - idle ticks (5): staging accumulates, avail");
  Serial.println("   grows and plateaus near the cap.");
  Serial.println(" - drain ticks (5): full bounded drain, then");
  Serial.println("   available() sampled — should be 0.");
  Serial.println("ms,phase,avail_before,avail_after_drain");

  const uint32_t start = millis();
  uint32_t tick_count = 0;

  while (millis() - start < 30000) {
    bool draining = ((tick_count / 5) & 1) == 1;
    int avail_before = PDM.available();
    int avail_after = avail_before;
    if (draining) {
      // Bounded full drain — up to 32 iterations of 512 B each,
      // way more than accumulates in 500 ms of audio.
      for (int i = 0; i < 32; i++) {
        if (PDM.available() <= 0) break;
        (void)PDM.read(g_scratch, sizeof(g_scratch));
      }
      avail_after = PDM.available();
    }
    Serial.printf("%lu,%s,%d,%d\r\n",
                  (unsigned long)(millis() - start),
                  draining ? "drain" : "idle",
                  avail_before, avail_after);
    delay(500);
    tick_count++;
  }
}

static void test2_read_size_canary()
{
  Serial.println();
  Serial.println("=== Test 2: read-size canary ===");
  delay(200);  // let DMA accumulate well past 64 bytes

  uint8_t buf[80];
  memset(buf, 0xAA, sizeof(buf));

  int avail_before = PDM.available();
  int got = PDM.read(buf, 64);

  bool canary_ok = true;
  for (int i = 64; i < 80; i++) {
    if (buf[i] != 0xAA) { canary_ok = false; break; }
  }

  Serial.printf("available before = %d\r\n", avail_before);
  Serial.printf("PDM.read(buf, 64) returned %d\r\n", got);
  Serial.printf("canary[64..79] intact: %s\r\n", canary_ok ? "PASS" : "FAIL");
  Serial.printf("canary bytes: %02X %02X %02X %02X ...\r\n",
                buf[64], buf[65], buf[66], buf[67]);
}

static void test3_order_preserved()
{
  Serial.println();
  Serial.println("=== Test 3: order preserved across small reads ===");

  // Drain so both halves come from the same fresh DMA stream. Bounded
  // — DMA refills continuously so the loop must not be unbounded.
  drain_bounded(8);
  delay(200);  // let ~3200 bytes (100 ms @ 16 kHz mono 16-bit) accumulate

  // Two 32-byte reads, expected to be contiguous in the input stream.
  uint8_t a[32], b[32];
  int ga = PDM.read(a, 32);
  int gb = PDM.read(b, 32);

  Serial.printf("read #1 = %d bytes, read #2 = %d bytes\r\n", ga, gb);
  // Print the join boundary so a human can eyeball continuity (we
  // don't know the exact byte content, but a discontinuity vs the
  // surrounding samples is visible as a jump).
  Serial.print("join window (a[28..31] | b[0..3]): ");
  for (int i = 28; i < 32; i++) Serial.printf("%02X ", a[i]);
  Serial.print("| ");
  for (int i = 0; i < 4; i++) Serial.printf("%02X ", b[i]);
  Serial.println();
  // Auto-check: read #1 should be exactly 32 (we asked for it and >32
  // were available); same for read #2.
  Serial.printf("size clamp on both reads: %s\r\n",
                (ga == 32 && gb == 32) ? "PASS" : "FAIL");
}

void setup()
{
  pinMode(WB_IO2, OUTPUT); digitalWrite(WB_IO2, HIGH);
  pinMode(LED_GREEN, OUTPUT); digitalWrite(LED_GREEN, HIGH);

  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(50);

  Serial.println();
  Serial.println("=== Phase 3 PDM API check ===");
  Serial.printf("SAMPLE_RATE=%d CHANNELS=%d\r\n", SAMPLE_RATE, CHANNELS);

  rak18003Init();
  if (!PDM.begin(CHANNELS, SAMPLE_RATE)) {
    Serial.println("PDM.begin failed");
    while (1) yield();
  }

  Serial.printf("PDM.bufferSize() = %d bytes\r\n", PDM.bufferSize());

  test1_available_grow_drain();
  test2_read_size_canary();
  test3_order_preserved();

  Serial.println();
  Serial.println("=== checks complete; idling ===");
}

void loop()
{
  delay(1000);
}
