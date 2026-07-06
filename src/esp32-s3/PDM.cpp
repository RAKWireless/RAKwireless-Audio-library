#if defined(_VARIANT_RAK3112_)  || defined(ARDUINO_RAKWIRELESS_RAK3112)   //rak3112

#include "PDM.h"
#include <string.h>

PDMClass PDM;
I2SClass I2S;


PDMClass::PDMClass()
  : _buffer_size(BUFFER_SIZE),
    _staging_pos(0),
    _staging_avail(0)
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

  _staging_pos = 0;
  _staging_avail = 0;
  return 1;
}

void PDMClass::end()
{
  _pdm_i2s.end();
  _staging_pos = 0;
  _staging_avail = 0;
}

void PDMClass::_pump()
{
  // Recycle: if the caller has consumed everything, reset the
  // cursors so we get a full contiguous fill window at [0..size).
  if (_staging_pos == _staging_avail) {
    _staging_pos = 0;
    _staging_avail = 0;
  } else if (_staging_pos > 0) {
    // Shift the unconsumed tail down so pump() can append at the end.
    // The staging buffer is small (a few KB), so memmove is cheap.
    size_t pending = _staging_avail - _staging_pos;
    memmove(_staging, _staging + _staging_pos, pending);
    _staging_pos = 0;
    _staging_avail = pending;
  }

  size_t room = sizeof(_staging) - _staging_avail;
  if (room == 0) return;   // caller isn't draining; don't over-buffer

  i2s_chan_handle_t h = _pdm_i2s.rxChan();
  if (h == NULL) return;

  size_t got = 0;
  // Non-blocking pull. Return code is ESP_OK or ESP_ERR_TIMEOUT; both
  // are fine — `got` reflects the partial fill. Anything else means
  // the channel isn't in a readable state; treat as zero.
  esp_err_t err = i2s_channel_read(h, _staging + _staging_avail, room,
                                   &got, 0);
  if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
    got = 0;
  }
  _staging_avail += got;
}

int PDMClass::available()
{
  _pump();
  return (int)(_staging_avail - _staging_pos);
}

int PDMClass::read(void* buffer, size_t size)
{
  _pump();
  size_t pending = _staging_avail - _staging_pos;
  if (pending == 0) return 0;
  size_t to_copy = (pending < size) ? pending : size;
  memcpy(buffer, _staging + _staging_pos, to_copy);
  _staging_pos += to_copy;
  return (int)to_copy;
}

#endif
