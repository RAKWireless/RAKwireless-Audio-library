#include "tpt29555.h"

TPT29555   tpt29555_1(0x23);
TPT29555   tpt29555_2(0x25);


void setup () {
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH); // Enable power supply.
  delay(300);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  Serial.begin (115200) ;
  // Initialize Serial for debug output
//  time_t timeout = millis();
//  while (!Serial)
//  {
//    if ((millis() - timeout) < 5000)
//    {
//      delay(100);
//    }
//    else
//    {
//      break;
//    }
//  }
  Serial.println("RAK18003 IO Expander Test!!!");
  delay(1000);

  if (!tpt29555_1.begin())
  {
    Serial.println("Do not find IO Expander Chip1");
    while (1)
      delay(500);
  }
  tpt29555_1.pinMode(0, OUTPUT);
  if (!tpt29555_2.begin())
  {
    Serial.println("Do not find IO Expander Chip2");
    while (1)
      delay(500);
  }
   tpt29555_2.pinMode(0, OUTPUT);
}

void loop () {
  tpt29555_1.digitalWrite(0, 0);
  delay(1000);
  tpt29555_1.digitalWrite(0, 1);
  delay(1000);    
}


void TPT29555GpioTest(uint8_t status)
{
  for (uint8_t pinCount = 0; pinCount < 15; pinCount++)
  {
    tpt29555_1.digitalWrite(pinCount, status);
    tpt29555_2.digitalWrite(pinCount, status);
  }
  for (uint8_t ppCount = 0; ppCount < 15; ppCount++)
  {
    tpt29555_1.digitalWrite(ppCount, status);
    tpt29555_2.digitalWrite(ppCount, status);
  }
 
}
