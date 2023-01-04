/**
 * @file PDMSerialPlotter.ino
 * @author rakwireless.com
 * @brief This example reads audio data from the PDM microphones, and prints
 * out the samples to the Serial console. The Serial Plotter built into the
 * Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)
 * @version 0.1
 * @date 2022-06-6
 * @copyright Copyright (c) 2022
 */
 
#include "audio.h"

void EventProcess(void)
{
  DSPG_USER.event = true;
}

void setup() {
  //pinMode(WB_IO1, INPUT);
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, LOW);  
  delay(1000);
  digitalWrite(WB_IO2, HIGH);
  Serial.begin(115200);
  while(!Serial);
  //config interrupt
  pinMode(FW_INT_PIN, INPUT);
  attachInterrupt(FW_INT_PIN, EventProcess, RISING);
  
  DSPG_USER.begin();
  delay(100);
  DSPG_USER.config();
  DSPG_USER.run();
  //begin to detect.
  DSPG_USER.mode(USE_CASE_BARGEIN_1MIC);
}

void loop() {
   if(DSPG_USER.event)
   {
      DSPG_USER.EventCallback();
   }
}
