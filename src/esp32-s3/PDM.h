
#ifndef _PDM_H_INCLUDED
#define _PDM_H_INCLUDED

#include <Arduino.h>
#include "ESP_I2S.h"

// Phase 2 (driver_updates_plan.md): default port mapping on ESP32-S3
//
//   PDM  RX  ->  I2S_NUM_0   (ESP32-S3 only supports PDM_RX on port 0)
//   STD I/O ->  I2S_NUM_1
//
// The mapping is enforced by ownership, not by an explicit port id:
// `PDMClass` owns a private `I2SClass _pdm_i2s` instance, and the
// library-wide global `I2S` is a *separate* `I2SClass`. ESP_I2S calls
// `i2s_new_channel` with `id = I2S_NUM_AUTO`, so the IDF picks the
// next available port for each instance. Because PDM_RX is only
// supported on I2S_NUM_0, that instance lands there; the STD instance
// then lands on I2S_NUM_1. The two peripherals run concurrently — a
// later `I2S.begin(I2S_MODE_STD, …)` no longer tears down the PDM
// path the way it did when both classes shared one global `I2S`.
class PDMClass
{
public:
  PDMClass();
  virtual ~PDMClass();
  int begin(int channels, long sampleRate);
  void end();
  // Reads at most `size` bytes into `buffer`. Returns the number of
  // bytes actually written. Excess DMA bytes are retained by the
  // ESP-IDF i2s_channel_read ring between calls, so repeated small
  // reads preserve sample order without dropping data.
  virtual int read(void* buffer, size_t size);

private:
  // Private I2S binding for the PDM_RX path. Constructed alongside
  // the global `I2S` below; the IDF assigns it I2S_NUM_0 because
  // PDM_RX is only supported there on ESP32-S3.
  I2SClass _pdm_i2s;
};

extern PDMClass PDM;
extern I2SClass I2S;

#endif
