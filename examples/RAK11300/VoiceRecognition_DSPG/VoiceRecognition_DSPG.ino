/**
 * @file VoiceRecognition_DSPG.ino
 * @author rakwireless.com
 * @brief This example shows that use the RAK18080 and RAK18003 to recognize the specific simple commands.
 * and show the recognized commands on the terminal. When the DSPG detected one command two LEDS of WisBase
 * will toggle light state.
 * @version 0.1
 * @date 2022-06-6
 * @copyright Copyright (c) 2022
 */
 
#include "audio.h"

char cmd_string[40]="";
int cmd_id = 0;
uint32_t trigger_count = 0;
volatile uint8_t int_flag = 0;
uint8_t led_state = 0;

void EventProcess(void);

void setup() {
  
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, LOW);  
  delay(1000);
  digitalWrite(WB_IO2, HIGH); 

  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
     
  Serial.begin(115200);
  while(!Serial);     

  Serial.println("waiting DSPG init ...");
  while(DSPG_USER.begin())
  {
    Serial.println("Please check !!!");
    delay(1000);
  }
  while(DSPG_USER.micCheck()==0) //Check if the microphone board is connected on the RAK18003
  {
    Serial.println("There is no microphone board, please check !"); 
    delay(1000);    
  }
  delay(100);
  //config interrupt
  DSPG_USER.detectedCallback(EventProcess);  
  DSPG_USER.readIoState();  //read the RAK18003 chip IO state
  Serial.printf("init ok\r\n");
  DSPG_USER.echoCommands();
  int_flag = 0; 
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_GREEN, HIGH);
}

void loop() {
   
  if(int_flag==1)
   {  
      trigger_count++;
      Serial.printf("***************** %d\r\n",trigger_count);
      uint16_t pin_state = DSPG_USER.readIoState();   //this is necessary to read IO expander register
      Serial.printf("pinstate:%04X\r\n",pin_state);
      DSPG_USER.eventCallBack(cmd_string,&cmd_id);
      Serial.printf("CMD ID: %d\r\n",cmd_id);
      Serial.println(cmd_string); 
      led_state = !led_state;
      
      digitalWrite(LED_BLUE, led_state);
      digitalWrite(LED_GREEN, led_state);
      
      int_flag = 0; //clear the interrupt flag
   }
}

void EventProcess(void)
{
  int_flag = 1;
}
