#ifndef __TAS2560_H__
#define __TAS2560_H__

#include <arduino.h>
#include <Wire.h>
#include "tas2560reg.h"

#define ALARM_PIN   WB_IO1

class TAS2560
{
public:

  TAS2560(uint8_t addr);
  TAS2560();
  bool begin(uint8_t addr,TwoWire &theWire); //Sets up the Hardware
  bool begin(uint8_t addr); //Sets up the Hardware
  bool begin(void);
  void writeRegister8(uint8_t reg, uint8_t value);
  void write(uint8_t reg, uint8_t *value,int length);
  uint8_t readRegister8(uint8_t reg);
  uint8_t read(uint8_t reg,uint8_t *data,uint8_t length);     
  void transmit_registers(cfg_reg *r, int n);
  void reset(void);  
  uint8_t set_pcm_word_length(uint8_t length);
  uint8_t get_pcm_word_length(void);
  uint8_t set_pcm_channel(ASI_channel mode);
  uint8_t get_pcm_channel(void);
  uint8_t set_gain(uint8_t gain); //Set the gain of the AMP, the setting range is 0-15dB default is 15.
  uint8_t get_gain(void);

  void set_mute(void);
  void set_alarm_pin(void(*function)(void));
  uint8_t get_power_status(void);
  uint8_t get_die_temp(void);
  uint16_t get_vbat(void);
  void set_alarm(void);
  uint8_t get_alarm(void);
  uint8_t set_speaker_load(uint8_t impedance);
  uint8_t get_speaker_load(void);
  uint8_t read_INT_DET1(void);
  uint8_t read_INT_DET2(void);
  unsigned int clear_interrupt(void);

  void clear_alarm(void);
  unsigned int read_alarm(void);

private:

  uint8_t i2c_addr; //the  deviceaddress
  TwoWire *_wire;   //the  i2c port interface
  int _alarm_pin;   //read alarm pin
  uint8_t volume;
  void write_page(void);
  void writeRegister(uint8_t registerAddress, uint8_t *writeData, uint8_t size);
  void readRegister(uint8_t registerAddress, uint8_t *readData, uint8_t size); 
};

#endif /* __TAS2560_H__ */
