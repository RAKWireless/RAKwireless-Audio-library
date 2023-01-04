#include "tpt29555.h"

TPT29555   tpt29555_1(0x23);


void setup () {
  pinMode(WB_IO1, INPUT);
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH); // Enable power supply.
  delay(300);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  Serial.begin (115200) ;

  Serial.println("RAK18003 IO Expander Test!!!");
  delay(1000);

  if (!tpt29555_1.begin())
  {
    Serial.println("Do not find IO Expander Chip1");
    while (1)
      delay(500);
  }
  tpt29555_1.pinMode(0, INPUT);
  attachInterrupt(WB_IO1,io_callback,CHANGE);
}

void loop () {
    
}

void io_callback(){
  //read the io register and judge
}
