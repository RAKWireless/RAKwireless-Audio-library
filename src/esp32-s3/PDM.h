
#ifndef _PDM_H_INCLUDED
#define _PDM_H_INCLUDED

#include <Arduino.h>
#include "ESP_I2S.h"


class PDMClass
{
public:
  PDMClass();
  virtual ~PDMClass();
  int begin(int channels, long sampleRate);
  void end();
  virtual int read(void* buffer, size_t size);

};

extern PDMClass PDM;
extern I2SClass I2S;

#endif
