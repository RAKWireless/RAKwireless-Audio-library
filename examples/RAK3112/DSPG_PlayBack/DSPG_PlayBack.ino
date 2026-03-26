/**
   @file DSPG_PlayBack.ino
   @author rakwireless.com
   @brief RAK3112 (ESP32-S3) port: voice recognition on RAK18080 (DSPG) with prompt playback on RAK18060.
   @details
     Voice models are loaded into the DBMD10L on RAK18080 via SPI from any supported host
     (RAK4631 / RAK11200 / RAK11300 / RAK3112). The model binaries in `src/hex/` are not tied to
     the MCU architecture; this sketch uses the same `DSPG_USER` flow as `examples/RAK11200/DSPG_PlayBack`.
   @note Hardware: RAK3112 + RAK18003 + RAK18080 + RAK18060 (+ PDM mic module). Use battery power if required by your base board.
   @note ESP32-S3: I2S TX uses `I2S.setPins` / `ESP_I2S` (already initialized inside `DSPG_USER.begin()` at 16 kHz stereo).
                  Playback path mirrors RAK11200 (no `TxIRQCallBack` / nRF `SoftwareTimer`).
   @version 0.2
   @date 2026-03-25
   @copyright Copyright (c) 2025-2026
*/

#include "audio.h"
#include "sound.h"

#define DSPG_USE_MODEL       MODEL_GROUP1
#define COMMAND_GROUP_CHOOSE 1

#define TRIGGER_STAGE_MS     8000u

TAS2560 AMP_Left;
TAS2560 AMP_Right;

#define AMP_LEFT_ADDRESS  0x4c
#define AMP_RIGTT_ADDRESS 0x4f

typedef struct
{
  int cmd_id;
  char cmd[60];
} dspg_event_t;

QueueHandle_t eventQueue;

char cmd_string[60] = "";
int cmd_id = 0;
uint32_t trigger_count = 0;
volatile uint8_t int_flag = 0;
uint8_t led_state = 0;
uint32_t trigger_stage_until_ms = 0;

void set_AMP_mute();
void set_AMP_unmute();
void EventProcess(void);
void speaker_task(void *pvParameters);

static bool is_trigger_word(const char *cmd)
{
  if (!cmd || !cmd[0])
  {
    return false;
  }
  for (int i = 0; i < 3; i++)
  {
    if (strcmp(cmd, cyberon_trigger_commands[i]) == 0)
    {
      return true;
    }
  }
  return false;
}

void setup()
{
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, LOW);
  delay(1000);
  digitalWrite(WB_IO2, HIGH);
  delay(500);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  time_t timeout = millis();
  Serial.begin(115200);
  while (!Serial)
  {
    if ((millis() - timeout) < 3000)
    {
      delay(100);
    }
    else
    {
      break;
    }
  }

  if (!AMP_Left.begin(AMP_LEFT_ADDRESS))
  {
    Serial.printf("TAS2560 left init failed\r\n");
    delay(500);
  }
  AMP_Left.set_pcm_channel(LeftMode);
  AMP_Left.set_gain(5);
  if (!AMP_Right.begin(AMP_RIGTT_ADDRESS))
  {
    Serial.printf("TAS2560 right init failed\r\n");
    delay(500);
  }
  AMP_Right.set_pcm_channel(RightMode);
  AMP_Right.set_gain(5);
  Serial.println("=====================================");
  set_AMP_mute();

  DSPG_USER.SetActiveCommandGroup(COMMAND_GROUP_CHOOSE);
  while (DSPG_USER.begin(DSPG_USE_MODEL, sizeof(DSPG_USE_MODEL)))
  {
    Serial.println("DSPG init failed — check RAK18003/RAK18080 wiring and power.");
    delay(500);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(500);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
  }
  while (DSPG_USER.micCheck() == 0)
  {
    Serial.println("Microphone board not detected on RAK18003.");
    delay(500);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(500);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
  }
  delay(100);

  DSPG_USER.detectedCallback(EventProcess);
  DSPG_USER.readIoState();
  Serial.printf("init ok\r\n");
  DSPG_USER.echoCommands(COMMAND_GROUP_CHOOSE);
  int_flag = 0;

  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, LOW);

  eventQueue = xQueueCreate(10, sizeof(dspg_event_t));
  if (!eventQueue)
  {
    Serial.println("create event queue failed");
  }
  /* Priority <= Arduino loop so loop() can service DSP INT while prompt plays. */
  if (!xTaskCreate(speaker_task, "speaker task", 4096, NULL, 1, 0))
  {
    Serial.println("creat speaker task failed");
  }
}

void loop()
{
  if (trigger_stage_until_ms != 0u && (int32_t)(millis() - trigger_stage_until_ms) >= 0)
  {
    trigger_stage_until_ms = 0u;
    Serial.println("DSPG Switch to Trigger Stage");
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
    led_state = 0;
  }

  if (int_flag == 1)
  {
    trigger_count++;
    Serial.printf("***************** %lu\r\n", (unsigned long)trigger_count);
    // Clear DSPG event first, then read IO expander ports to release its INT output.
    DSPG_USER.eventCallBack(cmd_string, &cmd_id);
    (void)DSPG_USER.readIoState();
    if (cmd_id == 0 && cmd_string[0] == '\0')
    {
      int_flag = 0;
      return;
    }
    Serial.printf("CMD ID: %d\r\n", cmd_id);
    Serial.println(cmd_string);
    led_state = !led_state;
    trigger_stage_until_ms = millis() + TRIGGER_STAGE_MS;
    digitalWrite(LED_GREEN, led_state);
    dspg_event_t ev;
    ev.cmd_id = cmd_id;
    strncpy(ev.cmd, cmd_string, sizeof(ev.cmd) - 1);
    ev.cmd[sizeof(ev.cmd) - 1] = '\0';
    xQueueSend(eventQueue, &ev, portMAX_DELAY);
    int_flag = 0;
  }
}

void IRAM_ATTR EventProcess(void)
{
  int_flag = 1;
}

void set_AMP_mute()
{
  AMP_Left.set_mute();
  AMP_Right.set_mute();
}

void set_AMP_unmute()
{
  AMP_Left.set_unmute();
  AMP_Right.set_unmute();
}

void speaker_task(void *pvParameters)
{
  (void)pvParameters;

  dspg_event_t ev;
  uint16_t write_buf[2];

  for (;;)
  {
    if (xQueueReceive(eventQueue, &ev, portMAX_DELAY) != pdPASS)
    {
      continue;
    }

    // Always print what the playback task received (helps debug missing follow-up recognition)
    Serial.printf("[DSPG] event in task: id=%d, cmd=%s\r\n", ev.cmd_id, ev.cmd);

    // Play prompt sound when wake/trigger word is detected.
    // Do NOT rely on fixed cmd_id range because IDs may differ between model groups.
    if (is_trigger_word(ev.cmd))
    {
      Serial.println("DSPG Switch to Command Stage");
      digitalWrite(LED_BLUE, HIGH);
      set_AMP_unmute();

      for (int i = 0; i < (int)(sizeof(sound_buff) / 2); i++)
      {
        int16_t left = (int16_t)((uint16_t)sound_buff[i * 2] | ((uint16_t)sound_buff[i * 2 + 1] << 8));
        write_buf[0] = (uint16_t)left;
        write_buf[1] = (uint16_t)left;
        (void)I2S.write((const uint8_t *)write_buf, sizeof(write_buf));
        if ((i & 0x3f) == 0)
        {
          yield();
        }
      }
      set_AMP_mute();
    }
  }
}
