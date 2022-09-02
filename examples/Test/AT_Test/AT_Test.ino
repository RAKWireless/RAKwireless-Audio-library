#include "audio.h"

uint32_t g_lpdwLicense[46];
String rev = "";
String rsp = "";


void setup() {
  Serial.begin(115200);
  while(!Serial);
  Audio_Handle.begin();
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
}

void loop() {
  at_process();
}


//AT+LICENSE=47444543056C09211FE66BBB80EF440D6DA31823921853A6AC40C7B95EAB5F8FA37DA271EEB06713ADA47BE560015ED072473DE0880C85F253DFBDFF71FD4DA19E1DD3515567AFF9261C7711A2EE1B35F99935ACDFA5480B3819A84656442D87FBF3787A5AD3E4516014D86FBECBA7F909CA4EC901B2537AA6046E728BD6EFEF1A32573FE74A7DA27CAF33EFF46C9440B403DABC9A394ADE4F0FFACD5601F3B7BC5C60DE267E97E7B5787F18BFADD6F70C1F042D26493FF4

void at_process()
{
  int timeout = 250;
  char cArr[256] = {0};
  char sn[32] = {0};
  while (timeout--)
  {
    if (Serial.available() > 0)
    {
      rev = Serial.readString();
    }
    delay(1);
  }  
  if(rev.equals("AT+SN?"))
  {
    Serial.write("AT+SN: get the serial number of the device.\nOK\n");
  }
  if(rev.equals("AT+SN=?"))
  {
    memset(sn, 0, 32);
    Audio_Handle.GetBoardId(sn);
    Serial.print(sn);
    Serial.write("\nOK\n");
  }
  if(rev.equals("AT+LICENSE?"))
  {
    Serial.write("AT+LICENSE: get the LICENSE from Cyberon.\nOK\n");
  }  
  if(rev.equals("AT+LICENSE=?"))
  {
    Audio_Handle.GetLicense(g_lpdwLicense);
    for(int i=0;i<46;i++)
    {
      Serial.print(g_lpdwLicense[i],HEX);
      Serial.print(" ");
    } 
    Serial.write("\nOK\n");    
  }  
  if(rev.indexOf("AT+LICENSE=") != -1 && rev.length() >= 379)
  {
    uint8_t tmp[184] = {0};
    char  tmp2[2];
    int index = 0;
    int index_l = 0;
    index = rev.indexOf("=");
    index++; 
    //length is 379
    for(int j = index; j<379 ;j++)
    {     
      tmp2[0]= rev[j];
      j++;
      tmp2[1]= rev[j];
      tmp[index_l++] = str_to_unit(tmp2); 
    }
    index_l = 0;
    for(int k = 0; k<184;k++)
    {
      g_lpdwLicense[index_l++] = ((uint32_t)tmp[k] << 24) | ((uint32_t)tmp[k+1] << 16) | ((uint32_t)tmp[k+2] << 8) | ((uint32_t)tmp[k+3]);
      k += 3; 
    }
    for(int i=0;i<46;i++)
    {
      Serial.print(g_lpdwLicense[i],HEX);
      Serial.print(" ");   
    }       
    Audio_Handle.StoreLicense(g_lpdwLicense);
    Serial.write("\nOK\n"); 
  }   
  rev = "";
}

uint8_t str_to_unit(char *s)
{
  uint8_t h = s[0];
  uint8_t l = s[1];  
  if(h < 'A')
  {
    h = h - '0';
  }
  else
  {
    if(h == 'A')
      h = 10;
    if(h == 'B')
      h = 11;
    if(h == 'C')
      h = 12;
    if(h == 'D')
      h = 13;
    if(h == 'E')
      h = 14;
    if(h == 'F')
      h = 15;     
  }
  
  if(l < 'A')
  {
    l = l - '0';
  }
  else
  {
    if(l == 'A')
      l = 10;
    if(l == 'B')
      l = 11;
    if(l == 'C')
      l = 12;
    if(l == 'D')
      l = 13;
    if(l == 'E')
      l = 14;
    if(l == 'F')
      l = 15;     
  }  

  return h*16+l;
}
