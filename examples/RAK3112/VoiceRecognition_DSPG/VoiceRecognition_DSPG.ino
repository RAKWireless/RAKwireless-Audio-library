/**
 * @file VoiceRecognition_DSPG.ino
 * @author rakwireless.com
 * @brief RAK3112 port: recognize voice commands using RAK18080 (DSPG) and print results.
 *
 * Principle (same as RAK4631/DSPG_PlayBack):
 * - Init DSPG + mic routing via `DSPG_USER.begin() / micCheck()`
 * - Register interrupt callback via `DSPG_USER.detectedCallback(EventProcess)`
 * - In main loop, when triggered, call `DSPG_USER.eventCallBack(cmd_string, &cmd_id)`
 *
 * Difference vs DSPG_PlayBack:
 * - This example ONLY prints recognized command id/string and toggles LEDs.
 * - It does NOT play prompt audio.
 *
 * @version 0.1
 * @date 2026-03-25
 * @copyright Copyright (c) 2026
 */

#include "audio.h"

#define DSPG_USE_MODEL        MODEL_GROUP1
#define COMMAND_GROUP_CHOOSE 1

// Must be >= 60 because DSPG library copies 60-byte command strings.
char cmd_string[60] = "";
int cmd_id = 0;
uint32_t trigger_count = 0;
volatile uint8_t int_flag = 0;
uint8_t led_state = 0;
uint32_t empty_evt_count = 0;

void EventProcess(void);

void setup()
{
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, LOW);
  delay(1000);
  digitalWrite(WB_IO2, HIGH);

  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);

  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }

  Serial.println("waiting DSPG init ...");

  DSPG_USER.SetActiveCommandGroup(COMMAND_GROUP_CHOOSE);
  while (DSPG_USER.begin(DSPG_USE_MODEL, sizeof(DSPG_USE_MODEL)))
  {
    Serial.println("Please check !!!");
    delay(1000);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(500);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
  }

  while (DSPG_USER.micCheck() == 0)
  {
    Serial.println("There is no microphone board, please check !");
    delay(1000);
  }

  delay(100);

  DSPG_USER.detectedCallback(EventProcess);
  DSPG_USER.readIoState();
  Serial.printf("init ok\r\n");
  DSPG_USER.echoCommands(COMMAND_GROUP_CHOOSE);

  int_flag = 0;
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_GREEN, HIGH);
}

void loop()
{
  if (int_flag == 1)
  {
    trigger_count++;
    Serial.printf("***************** %lu\r\n", (unsigned long)trigger_count);

    // Clear DSPG event first, then read IO expander ports to release its INT output.
    cmd_id = 0;
    cmd_string[0] = '\0';
    DSPG_USER.eventCallBack(cmd_string, &cmd_id);
    uint16_t pin_state_1 = DSPG_USER.readIoState();
    delay(2);
    uint16_t pin_state_2 = DSPG_USER.readIoState();
    Serial.printf("pinstate:%04X -> %04X\r\n", pin_state_1, pin_state_2);

    if (cmd_id == 0 && cmd_string[0] == '\0')
    {
      // No VT event was latched for this interrupt (can happen due to edge/noise).
      // Keep one short line so it doesn't look "stuck", without dumping verbose DSP logs.
      empty_evt_count++;
      Serial.printf("No command detected (%lu)\r\n", (unsigned long)empty_evt_count);
      int_flag = 0;
      return;
    }

    Serial.printf("CMD ID: %d\r\n", cmd_id);
    Serial.println(cmd_string);

    led_state = !led_state;
    digitalWrite(LED_BLUE, led_state);
    digitalWrite(LED_GREEN, led_state);

    int_flag = 0;
  }
}

void EventProcess(void)
{
  int_flag = 1;
}

