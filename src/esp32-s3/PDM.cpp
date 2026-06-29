#if defined(_VARIANT_RAK3112_)  || defined(ARDUINO_RAKWIRELESS_RAK3112)   //rak3112

#include "PDM.h"

PDMClass PDM;
I2SClass I2S;


PDMClass::PDMClass()
  : _buffer_size(BUFFER_SIZE)
{
}

PDMClass::~PDMClass()
{

}



int PDMClass::begin(int channels, long sampleRate)
{
  // PDM clock on GPIO 42, PDM data on GPIO 41 (RAK18030 mic via RAK18003).
  _pdm_i2s.setPinsPdmRx(42, 41);

  // Bring up PDM_RX on the private instance — IDF assigns I2S_NUM_0
  // (the only port that supports PDM_RX on ESP32-S3). The library-
  // wide global `I2S` is a separate instance that takes I2S_NUM_1 for
  // STD output; the two now coexist instead of overwriting each other.
  if (!_pdm_i2s.begin(I2S_MODE_PDM_RX, sampleRate, I2S_DATA_BIT_WIDTH_16BIT,
                      (i2s_slot_mode_t)channels))
  {
    Serial.println("Failed to initialize I2S!");
    return 0;
  }

  return 1;
}

void PDMClass::end()
{
  _pdm_i2s.end();
}


int PDMClass::available()
{
  // ESP_I2S::available() reports BYTES present in the IDF DMA ring
  // for the RX channel. Forward unchanged — Arduino's nRF52 PDM
  // library uses the same byte semantics, so cross-core code can call
  // PDM.available() / PDM.read() without conditional sizing.
  return _pdm_i2s.available();
}


int PDMClass::read(void* buffer, size_t size)
{
  // Phase 2 step 4: honor `size`. The previous implementation read
  // `available()` bytes regardless of the caller's buffer length,
  // which silently overran small buffers. Clamping to `size` is safe
  // because i2s_channel_read leaves any unread bytes in the IDF DMA
  // ring for the next call — repeated small reads preserve sample
  // order without dropping data.
  size_t available_bytes = _pdm_i2s.available();
  if (available_bytes == 0) {
    return 0;
  }
  size_t to_read = (available_bytes < size) ? available_bytes : size;
  return _pdm_i2s.readBytes((char *)buffer, to_read);
}

#endif
