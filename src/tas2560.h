#ifndef __TAS2560_H__
#define __TAS2560_H__

#include <Arduino.h>
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
  // Rate-aware overloads. The static init table boots the part for the
  // 16/24 kHz PCM window (PCM_RATE = 0x32) with every clock-error flag
  // enabled (CLK_ERR_2 = 0x3F); at 8 kHz the part flags a clock error
  // and self-mutes. These overloads run the static init and then
  // overwrite PCM_RATE (and a rate-appropriate CLK_ERR_2) so the
  // configured I2S BCLK actually drives audio.
  bool begin(uint8_t addr, uint32_t sample_rate);
  bool begin(uint8_t addr, TwoWire &theWire, uint32_t sample_rate);
  // Programs PCM_RATE (book 0x00 page 0x00 reg 0x36) for the supplied
  // sample rate. Returns false for unsupported rates without touching
  // the register. Supported: 8000, 12000, 16000, 24000, 32000, 48000,
  // 44100, 88200, 96000.
  bool set_sample_rate(uint32_t fs);
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
  void set_unmute(void);
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

  // Debug-only register read helpers — see TAS2560_DEBUG_REGISTERS.
  // These are guarded behind a non-release define so production builds
  // do not expose register-poking APIs. They restore book/page state.
#ifdef TAS2560_DEBUG_REGISTERS
  uint8_t debug_read_book_page_reg(uint8_t book, uint8_t page, uint8_t reg);
  // Debug-only: loosen the CLK_ERR_2 mask to help diagnose 8 kHz clock
  // errors. Release builds intentionally do not ship a public API for
  // this — keep it behind the debug guard.
  void    debug_set_clk_err_mask(uint8_t mask);
#endif

private:

  uint8_t i2c_addr; //the  deviceaddress
  uint32_t _sample_rate = 16000; // last rate configured via set_sample_rate
  void apply_rate_dependent_init(uint32_t fs);
  TwoWire *_wire;   //the  i2c port interface
  int _alarm_pin;   //read alarm pin
  uint8_t volume;
  void write_page(void);
  void writeRegister(uint8_t registerAddress, uint8_t *writeData, uint8_t size);
  void readRegister(uint8_t registerAddress, uint8_t *readData, uint8_t size); 
};

#endif /* __TAS2560_H__ */
