#if defined(_VARIANT_RAK3112_)  || defined(ARDUINO_RAKWIRELESS_RAK3112)   //rak3112

#include "PDM.h"
// #include "esp_now.h"
// #include "driver/i2s.h"

PDMClass PDM;
I2SClass I2S;


PDMClass::PDMClass()
{

}

PDMClass::~PDMClass()
{

}



int PDMClass::begin(int channels, long sampleRate)
{
  // setup 42 PDM clock and 41 PDM data pins
  I2S.setPinsPdmRx(42, 41); 

  // start I2S at 16 kHz with 16-bits per sample
  if (!I2S.begin(I2S_MODE_PDM_RX, sampleRate, I2S_DATA_BIT_WIDTH_16BIT, (i2s_slot_mode_t)channels)) 
  {
    Serial.println("Failed to initialize I2S!");
  }

	return 1;
}

void PDMClass::end()
{
  I2S.end();
}


int PDMClass::read(void* buffer, size_t size)
{
  size_t available_bytes, read_bytes;

  available_bytes = I2S.available();

  read_bytes = 0;
  read_bytes = I2S.readBytes((char *)buffer, available_bytes);

  return read_bytes;
}

#endif
