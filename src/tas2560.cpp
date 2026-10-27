#include "tas2560.h"
#include "tas2560reg.h"
#include <Arduino.h>
#include <Wire.h>
#include "SoftwareIIC.h"

IOIIC amp_i2c;

TAS2560::TAS2560(uint8_t addr)
{
  i2c_addr = addr;
  #if defined(_VARIANT_RAK11200_)

  #else
  _wire = &Wire; 
  #endif  
}
TAS2560::TAS2560()
{
  // i2c_addr = 0x4C;
  #if defined(_VARIANT_RAK11200_)

  #else
  _wire = &Wire;  
  #endif  
}
bool TAS2560::begin(uint8_t addr,TwoWire &theWire) //Sets up the Hardware
{
  i2c_addr = addr;
  #if defined(_VARIANT_RAK11200_)
  amp_i2c.set_pin(SDA,SCL);
   if(amp_i2c.begin(i2c_addr))
   {
    transmit_registers(registers,sizeof(registers)/2); 
    set_alarm();
    set_pcm_word_length(PCM_LEN16);  // 16-bit samples; reset default is 24-bit
    _sync_power_state();
    _rate_aware = false;
    return true;
   } 
  #else
  _wire = &theWire;
  _wire->begin();
  // A basic scanner, see if it ACK's
  _wire->beginTransmission(i2c_addr);
  if (_wire->endTransmission() == 0) {
    
    transmit_registers(registers,sizeof(registers)/2);
    set_alarm();
    set_pcm_word_length(PCM_LEN16);  // 16-bit samples; reset default is 24-bit
    _sync_power_state();
    _rate_aware = false;
    return true;
  }
  #endif
  return false;
}
bool TAS2560::begin(uint8_t addr) //Sets up the Hardware
{
  i2c_addr = addr;
  #if defined(_VARIANT_RAK11200_)  
   amp_i2c.set_pin(SDA,SCL);
   if(amp_i2c.begin(i2c_addr))
   {
    transmit_registers(registers,sizeof(registers)/2);  
    set_alarm();    
    set_pcm_word_length(PCM_LEN16);  // 16-bit samples; reset default is 24-bit
    _sync_power_state();
    _rate_aware = false;
    return true;
   }  
  #else
  _wire = &Wire;
  _wire->begin();
  // A basic scanner, see if it ACK's
  _wire->beginTransmission(i2c_addr);
  if (_wire->endTransmission() == 0) {
    
    transmit_registers(registers,sizeof(registers)/2); 
    set_alarm();
    set_pcm_word_length(PCM_LEN16);  // 16-bit samples; reset default is 24-bit
    _sync_power_state();
    _rate_aware = false;
    return true;
  } 
  #endif  
  return false;
}
// Rate-aware begin: fetch the per-rate register table via
// tas2560_dsp_coeffs_for_rate() and send THAT instead of the stock
// `registers[]`. Falls back to the stock table when the rate has no
// specific tuning. Then apply_rate_dependent_init() runs to override
// PLL / clock-error registers for the actual BCLK on our hardware.
bool TAS2560::begin(uint8_t addr, uint32_t sample_rate)
{
  i2c_addr = addr;
#if defined(_VARIANT_RAK11200_)
  amp_i2c.set_pin(SDA, SCL);
  if (!amp_i2c.begin(i2c_addr)) return false;
#else
  _wire = &Wire;
  _wire->begin();
  _wire->beginTransmission(i2c_addr);
  if (_wire->endTransmission() != 0) return false;
#endif
  _sample_rate = sample_rate;
  _rate_aware = true;
  _reload_for_current_rate();
  return true;
}
bool TAS2560::begin(uint8_t addr, TwoWire &theWire, uint32_t sample_rate)
{
  i2c_addr = addr;
#if defined(_VARIANT_RAK11200_)
  amp_i2c.set_pin(SDA, SCL);
  if (!amp_i2c.begin(i2c_addr)) return false;
#else
  _wire = &theWire;
  _wire->begin();
  _wire->beginTransmission(i2c_addr);
  if (_wire->endTransmission() != 0) return false;
#endif
  _sample_rate = sample_rate;
  _rate_aware = true;
  _reload_for_current_rate();
  return true;
}

// PCM_RATE (book 0x00 page 0x00 reg 0x36) nibble values per the
// TAS2560 datasheet PCM_RATE table (Table 95).
//
// 8 kHz   -> 0x12
// 12 kHz  -> 0x22
// 16/24   -> 0x32
// 32/48   -> 0x42
// 44.1/88.2 -> 0x52
// 96 kHz  -> 0x62
bool TAS2560::set_sample_rate(uint32_t fs)
{
  // PCM_RATE (book 0x00 page 0x00 reg 0x36). Per the TAS2560
  // datasheet (SLASE86E, Table 95), the field is 2 bits wide at
  // [1:0]; upper bits are reserved with bits [5:4] hardcoded to 3h.
  // Register value = 0x30 | rate_code:
  //   0 = 8 kHz    -> 0x30
  //   1 = 16 kHz   -> 0x31
  //   2 = 48 kHz   -> 0x32
  //   3 = 96 kHz   -> 0x33
  //
  // The chip has exactly these four rates. 44.1 / 88.2 kHz share
  // no separate mode — they fall through to the 48 / 96 kHz slot
  // and the chip tolerates the slight LRCLK mismatch (works
  // empirically for WAV playback). 24 / 32 kHz aren't officially
  // supported at all; we map them to the closest available mode
  // rather than return false so common codec rates still play.
  uint8_t pcm_rate;
  switch (fs) {
    // 16 kHz uses the 48 kHz mode, not its native 0x31: the register
    // table loaded at >= 16 kHz carries a DSP patch tuned for 48 kHz
    // mode, and running it in 16 kHz mode gave audible hiss in silent
    // passages on RAK18060. 0x32 is also what the stock table uses at
    // 16 kHz. Playback speed follows the host WCLK either way. A PPC3
    // export for 16 kHz (as done for 8 kHz) would allow the native mode.
    case 8000:                                  pcm_rate = 0x30; break;
    case 12000:                                 pcm_rate = 0x31; break;
    case 16000: case 24000:
    case 32000: case 44100: case 48000:         pcm_rate = 0x32; break;
    case 88200: case 96000:                     pcm_rate = 0x33; break;
    default: return false;
  }
  // PCM_RATE is on book 0, page 0.
  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, 0x00);
  writeRegister8(TAS25X_PCM_RATE, pcm_rate);
  _sample_rate = fs;
  return true;
}

// Send the register table for the rate-aware begin() rate (or the
// stock table after a legacy begin()) and re-apply everything that
// follows a table load. Used by begin() and by the alarm-recovery paths,
// which in 1.0.x always reloaded the stock 16/48 kHz table and so
// would reconfigure an 8 kHz amp for the wrong rate.
void TAS2560::_reload_for_current_rate(void)
{
  if (_rate_aware) {
    int count = 0;
    const cfg_reg *table = tas2560_dsp_coeffs_for_rate(_sample_rate, &count);
    transmit_registers(const_cast<cfg_reg *>(table), count);
    set_alarm();
    apply_rate_dependent_init(_sample_rate);
  } else {
    transmit_registers(registers, sizeof(registers)/2);
    set_alarm();
    set_pcm_word_length(PCM_LEN16);
    _sync_power_state();
  }
}

void TAS2560::apply_rate_dependent_init(uint32_t fs)
{
  // PCM_RATE is only written for rates with a native or sibling mode.
  // Other rates (e.g. 22 050 Hz) keep the table's PCM_RATE but still get
  // the PLL settings below, which are valid for any rate.
  set_sample_rate(fs);
  _sample_rate = fs;

  // PLL: PLL_CLK = PLL_CLKIN * J.D / P (datasheet SLASE86E 9.3.7), with
  // PLL_CLKIN = BCLK and P = 1 from the register tables. This library's
  // I2SEventClass clocks two slots per frame, 16-bit at >= 16 kHz and
  // 32-bit below (BCLK widening), so BCLK = 32 * fs or 64 * fs.
  //
  // These values are the ones verified on RAK18060 hardware, NOT the
  // datasheet rules, which failed on the bench:
  //   - 9.3.8 says PLL_CLK = 1024 * Fs. At 8 kHz that is J = 16
  //     (8.192 MHz): silent twice, with PLL_LOWF clear and set.
  //     J = 96 (49.152 MHz, the 48 kHz-family clock) plays cleanly, as
  //     does every rate >= 44.1 kHz at 1024 * Fs. The datasheet's own
  //     example script runs the PLL at 12.288 MHz MCLK * default J 4 =
  //     49.152 MHz, so the chip appears to want the 48 kHz-family clock
  //     whatever the PCM rate. J = 96 is above the documented J range
  //     (1..63) but is the only value that has worked at 8 kHz.
  //   - Table 4 says set PLL_LOWF when the PLL input is below 1 MHz.
  //     Every clean configuration so far had it clear. Left clear.
  //   - 16 kHz also has a 512 kHz BCLK. The stock J = 32 runs it at
  //     16.384 MHz, and with that one amp hissed during playback (the
  //     hiss stayed on the same speaker when the channels were swapped,
  //     so it isn't in the source). J = 96 gives 49.152 MHz, the same
  //     internal clock as 48 kHz, which plays clean.
  // PLL_CLK at the other rates: 44.1 kHz 45.158 MHz, 48 kHz 49.152 MHz,
  // 96 kHz 98.304 MHz (J = 32, i.e. 1024 * Fs).
  const uint8_t pll_j = (fs <= 16000) ? 0x60 : 0x20;

  // Clock-error detection stays at the chip defaults at every rate:
  // with BCLK in the PLL's input range, its halt detection is
  // trustworthy and gives auto-mute when I2S stops.
  const uint8_t clk_err_1 = 0x03;
  const uint8_t clk_err_2 = 0x3F;

  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, 0x00);
  writeRegister8(TAS25X_PLL_JVAL, pll_j);
  writeRegister8(TAS25X_PLL_DVAL_1, 0x00);
  writeRegister8(TAS25X_PLL_DVAL_2, 0x00);
  writeRegister8(TAS25X_CLK_ERR_1, clk_err_1);
  writeRegister8(TAS25X_CLK_ERR_2, clk_err_2);

  // ASI word length: the register reset value is 24-bit (ASI_FORMAT =
  // 0x02) and no register table sets it, but this library always sends
  // 16-bit samples. Older set_pcm_channel(LeftMode) wrote 0x00 here by
  // accident (see set_pcm_channel), which is the only reason left-
  // channel playback had the right word length. Set it explicitly.
  set_pcm_word_length(PCM_LEN16);
  _sync_power_state();
}

// Called after every register-table load: the part comes out of the
// table unmuted, in whatever PWR_DEV mode the table's last PWR_CTRL_1
// write chose (stock table 0x80 = without boost; PPC3 8 kHz / 96 kHz
// tables 0x40 = with boost). Record that so set_unmute() restores it.
void TAS2560::_sync_power_state(void)
{
  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, 0x00);
  uint8_t pwr_dev = readRegister8(TAS25X_PWR_CTRL_1) & 0xC0;
  if (pwr_dev == 0x40 || pwr_dev == 0x80) _pwr_state = pwr_dev;
  _muted = false;
}

#ifdef TAS2560_DEBUG_REGISTERS
uint8_t TAS2560::debug_read_book_page_reg(uint8_t book, uint8_t page, uint8_t reg)
{
  // Save current book/page so we can restore it. The driver convention
  // is that book/page live at 0x7F and 0x00 of whatever the chip is
  // currently selected to, so a naive readback is safe enough here.
  uint8_t prev_page = readRegister8(TAS25X_PAGE);
  uint8_t prev_book = readRegister8(TAS25X_BOOK);

  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, book);
  writeRegister8(TAS25X_PAGE, page);
  uint8_t value = readRegister8(reg);

  // Restore.
  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, prev_book);
  writeRegister8(TAS25X_PAGE, prev_page);
  return value;
}

void TAS2560::debug_set_clk_err_mask(uint8_t mask)
{
  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, 0x00);
  writeRegister8(TAS25X_CLK_ERR_2, mask);
}
#endif // TAS2560_DEBUG_REGISTERS

bool TAS2560::begin(void) //Sets up the Hardware
{
  #if defined(_VARIANT_RAK11200_)
   amp_i2c.set_pin(SDA,SCL);
   if(amp_i2c.begin(i2c_addr));
   {
    transmit_registers(registers,sizeof(registers)/2);    
    set_alarm();
    set_pcm_word_length(PCM_LEN16);  // 16-bit samples; reset default is 24-bit
    _sync_power_state();
    _rate_aware = false;
    return true;
   }     
  #else
  _wire = &Wire;
  _wire->begin();
  // A basic scanner, see if it ACK's
  _wire->beginTransmission(i2c_addr);
  if (_wire->endTransmission() == 0) {
    
    transmit_registers(registers,sizeof(registers)/2); 
    set_alarm();
    set_pcm_word_length(PCM_LEN16);  // 16-bit samples; reset default is 24-bit
    _sync_power_state();
    _rate_aware = false;
    return true;
  }  
  #endif 
  return false; 
}
/*!
 *  @brief  I2c bus write.
 *  @param  registerAddress : Register address.
 *  @param  writeData		    : Write data pointer.
 *  @param  size			      : The length of the written data.
 */
void TAS2560::writeRegister(uint8_t registerAddress, uint8_t *writeData, uint8_t size)
{
  amp_i2c.IIC_Start();  
  amp_i2c.IIC_Send_Byte( (i2c_addr<<1)&0xFE);
  amp_i2c.IIC_Wait_Ack();
//  if(amp_i2c.IIC_Wait_Ack())
//    return NACK;    
  amp_i2c.IIC_Send_Byte( registerAddress );  
  amp_i2c.IIC_Wait_Ack();     
//  if(amp_i2c.IIC_Wait_Ack())
//    return NACK;
  for(uint8_t i = 0; i < size; i++)
  {      
    amp_i2c.IIC_Send_Byte( writeData[i] ); 
    amp_i2c.IIC_Wait_Ack();   
  }   
//  if(amp_i2c.IIC_Wait_Ack())
//    return NACK;
  amp_i2c.IIC_Stop();
//  return ACK; 

}

/*!
 *  @brief  I2c bus read.
 *  @param  registerAddress : Register address.
 *  @param  readData		    : Read data pointer.
 *  @param  size			      : The length of the written data.
 */
void TAS2560::readRegister(uint8_t registerAddress, uint8_t *readData, uint8_t size) 
{ 
  amp_i2c.IIC_Start();  
  amp_i2c.IIC_Send_Byte( (i2c_addr<<1)&0xfe );
  amp_i2c.IIC_Wait_Ack();
//  if(amp_i2c.IIC_Wait_Ack())
//    return NACK;
  amp_i2c.IIC_Send_Byte( registerAddress );   
  amp_i2c.IIC_Wait_Ack();   
//  if(amp_i2c.IIC_Wait_Ack())
//    return NACK;
  amp_i2c.IIC_Stop();
  delayMicroseconds(DELYA); 
  amp_i2c.IIC_Start();
  amp_i2c.IIC_Send_Byte( (i2c_addr<<1)|0x01 );
  amp_i2c.IIC_Wait_Ack();
//  if(amp_i2c.IIC_Wait_Ack())
//    return NACK; 
    
  for(uint8_t i = 0; i < size; i++)
  {    
    readData[i] = amp_i2c.IIC_Read_Byte(0);
  }        
  amp_i2c.IIC_Stop();

}

void TAS2560::writeRegister8(uint8_t reg, uint8_t value)  
{  
  #if defined(_VARIANT_RAK11200_)
  uint8_t data = value;
  writeRegister((uint8_t)reg,&data,1);
  #else
  _wire->beginTransmission(i2c_addr);
  _wire->write((uint8_t)reg);
  _wire->write((uint8_t)value);  
  _wire->endTransmission(); 
  #endif
}
void TAS2560::write(uint8_t reg, uint8_t *value,int length)  
{  
  int i=0;
  #if defined(_VARIANT_RAK11200_)
  writeRegister(reg,value,length);
  #else   
    _wire->beginTransmission(i2c_addr);
    _wire->write((uint8_t)reg);
    while(i<length)
    {
    _wire->write((uint8_t)value[i]);
    i++;  
    }  
    _wire->endTransmission(); 
   #endif  
}
uint8_t TAS2560::readRegister8(uint8_t reg) 
{ 
   uint8_t data=0;
  #if defined(_VARIANT_RAK11200_)
  uint8_t reg_addr = reg;
   readRegister(reg_addr, &data,1); 
  #else
    _wire->beginTransmission(i2c_addr);
    _wire->write((uint8_t)reg);
    _wire->endTransmission();
    _wire->requestFrom(i2c_addr,1); 
    data = (uint8_t)_wire->read();     
  #endif  
  return data;
}
uint8_t TAS2560::read(uint8_t reg,uint8_t *data,uint8_t length) 
{ 
  int i = 0;
  #if defined(_VARIANT_RAK11200_)
   readRegister(reg,data,length);
  #else
   _wire->beginTransmission(i2c_addr);
    _wire->write((uint8_t)reg);
    _wire->endTransmission();
    _wire->requestFrom(i2c_addr,length); 
    while(i<length)
    {
      data[i] = (uint8_t)_wire->read();  
      i++;
    }
  #endif   
  return i;
}
/**
 * @brief reset the tas2560 chip
 * @param void
 * @return none
 */
void TAS2560::reset(void)
{
  writeRegister8(TAS25X_PAGE,0);
  writeRegister8(TAS25X_RESET,1);   //reset chip
  delay(5);
}

/**
 * @brief set ASI PCM input word length
 * @param length:the length
 *  the length can be : 
 *  0:16bits  1:20bits  2:24bits  3:32bits  
 * @return the value of to be write the register
 */
uint8_t TAS2560::set_pcm_word_length(uint8_t length)
{
  uint8_t reg = readRegister8(TAS25X_ASI_FORMAT);
  reg = (reg&0xFC)|(length&0x03);
  writeRegister8(TAS25X_ASI_FORMAT,reg);
  return reg;
}
/**
 * @brief get ASI PCM input word length
 * @param void  
 * @return the value of the PCM input word length
 *  0:16bits  1:20bits  2:24bits  3:32bits  
 */
uint8_t TAS2560::get_pcm_word_length(void)
{
  uint8_t reg = readRegister8(TAS25X_ASI_FORMAT);
//  Serial.printf("read reg:%0X\r\n",reg);
  reg = (reg&0x03);
  switch(reg)
  {
   case PCM_LEN16 :  
    return 16;
    break;
   case PCM_LEN20 :   
    return 20;
    break;
   case PCM_LEN24 :    
    return 24;
    break;
   case PCM_LEN32 :    
    return 32;
    break;  
   default: break;
  }
  return 0;
}
/**
 * @brief set ASI PCM Channel Mode
 * @param mode:  the value can be 
 * PCM_CH_LEFT   0:left mode(default), 
 * PCM_CH_RIGHT  1:right mode, 
 * PCM_CH_STEREO 2:(left+right)/2 mode, 
 * PCM_CH_MONO   3:monoPCM mode         
 * @return the value of write to the register ASI_CHANNEL
 */
uint8_t TAS2560::set_pcm_channel(ASI_channel mode)
{
  // ASI_CHAN_MODE is ASI_CHANNEL (0x15) bits [1:0] (datasheet Table 73).
  // Version 1.0.x wrote the result to ASI_FORMAT (0x14) instead,
  // so the channel never changed (the register tables leave 0x15 at
  // 0x02, (L+R)/2) and the mode value landed in ASI_LENGTH: LeftMode
  // gave 16-bit, RightMode gave 20-bit.
  //
  // 96 kHz ("Mode 2 96k", datasheet 9.5.5.3) bypasses the processing
  // that applies ASI_CHAN_MODE: on RAK18060 an amp set to RightMode still
  // played the left slot. There, select the right slot by inverting WCLK
  // instead (ASI_CFG_1.ASI_WCLKE, Table 76) and reading the "left"
  // channel, which with WCLK inverted is the right slot. Other rates
  // clear the inversion so a later rate change can't leave it set.
  // Remembered so alarm recovery (which reloads the register table)
  // can put the channel back.
  _channel_mode = mode;
  _channel_set = true;
  const bool rate_96k = _rate_aware && _sample_rate >= 88200;
  uint8_t chan = mode & 0x03;
  uint8_t cfg = readRegister8(TAS25X_ASI_CFG_1) & ~ASI_CFG_1_WCLKE;
  if (rate_96k && chan == RightMode) {
    chan = LeftMode;
    cfg |= ASI_CFG_1_WCLKE;
  }
  writeRegister8(TAS25X_ASI_CFG_1, cfg);

  uint8_t reg = readRegister8(TAS25X_ASI_CHANNEL);
  reg = (reg&0xFC)|chan;
  writeRegister8(TAS25X_ASI_CHANNEL,reg);
  return reg;
}
/**
 * @brief get ASI PCM Channel Mode
 * @param void   
 * @return the value of the register ASI_CHANNEL's channel mode bits
 * 0:left mode(default),  1:right mode,  2:(left+right)/2 mode,   3:monoPCM mode
 */
uint8_t TAS2560::get_pcm_channel(void)
{
  uint8_t reg = readRegister8(TAS25X_ASI_CHANNEL);
  reg = (reg&0x03);
  // At 96 kHz, RightMode is implemented as LeftMode + inverted WCLK
  // (see set_pcm_channel); report it as RightMode.
  if (reg == LeftMode && (readRegister8(TAS25X_ASI_CFG_1) & ASI_CFG_1_WCLKE))
    reg = RightMode;
  return reg;
}
/**
 * @brief set the set volume
 * @param uint8_t gain can set from 0-15
 * @return the register of TAS25X_SPK_CTRL
 */
uint8_t TAS2560::set_gain(uint8_t gain)
{
  // While muted, only remember the gain: the mute works by holding the
  // DAC gain at 0, so writing it now would make the amp audible again.
  // set_unmute() applies the remembered gain.
  if (_muted) {
    volume = gain & 0x0f;
    return (readRegister8(TAS25X_SPK_CTRL) & ~0x0f) | volume;
  }
  return _write_gain(gain);
}
uint8_t TAS2560::_write_gain(uint8_t gain)
{
  uint8_t reg = readRegister8(TAS25X_SPK_CTRL);
  reg = reg&(~0x0f);
  reg = reg | (gain&0x0f);
  writeRegister8(TAS25X_SPK_CTRL,reg);
  return reg;
}
/**
 * @brief get the tas2560 volume
 * @param void
 * @return the register of TAS25X_SPK_CTRL
 */
uint8_t TAS2560::get_gain(void)
{
  // While muted the register holds 0; report the gain set_unmute() will
  // restore.
  if (_muted) return volume;
  uint8_t reg = readRegister8(TAS25X_SPK_CTRL);  
  reg = reg&0x0f;
  return reg;
}
/**
 * @brief write configuration list to register
 * @param cfg_reg *r, int n
 * @return none
 */
//   transmit_registers(registers, sizeof(registers)/sizeof(registers[0]));
void TAS2560::transmit_registers(cfg_reg *r, int n)
{
  int i = 0;
  while (i < n) {
      switch (r[i].command) {
      case CFG_META_SWITCH:
          // Used in legacy applications.  Ignored here.
          break;
      case CFG_META_DELAY:
          delay(r[i].param);
          break;
      case CFG_META_BURST: {
          // PPC3 burst format (see PPC3 exported reference code):
          //   r[i].param  = number of DATA bytes (not including the
          //                 leading start-register byte).
          //   Payload bytes: [start_reg, data0, data1, ..., data_{param-1}]
          //                 = 1 + param bytes total.
          //   These occupy ceil((1 + param) / 2) cfg_reg entries after
          //   the header, with padding in the last entry if odd.
          //   Advance the index past header + payload = 1 + (param/2)+1
          //   entries; the loop's trailing i++ contributes one, so
          //   we add (param / 2) + 1 here to match the PPC3 reference
          //   `i += (r[i].param / 2) + 1;`.
          uint8_t param = r[i].param;
          int payload_entries = (param / 2) + 1;
          if (param == 0 || (i + 1 + payload_entries) > n) {
            break;   // malformed — skip safely
          }
          uint8_t *payload = (uint8_t *)&r[i + 1];
          uint8_t start_reg = payload[0];
          write(start_reg, payload + 1, param);
          i += payload_entries;   // loop's i++ takes final step past burst
          break;
      }
      default:
            writeRegister8(r[i].command,r[i].param);
          break;
      }
      i++;
   }
}
/**
 * @brief get the tas2560 chip power status
 * @param void
 * @return uint8_t reg: the value of TAS25X_STATUS_POWER register
 */
uint8_t TAS2560::get_power_status(void)
{
  uint8_t reg = readRegister8(TAS25X_STATUS_POWER);
  return reg;
}
/**
 * @brief config the chip error alarm pin
 * @param void(*function)(void), error alarm callback function.
 * @return void
 */
void TAS2560::set_alarm_pin(void(*function)(void))
{
  _alarm_pin = ALARM_PIN;
  pinMode(_alarm_pin, INPUT_PULLUP); 
  /*
    LOW low level trigger
    CHANGE level change trigger
    RISING low-to-high trigger
    FALLING high-to-low trigger
  */
  attachInterrupt(digitalPinToInterrupt(_alarm_pin), function, RISING);//
}
/**
 * @brief get the chip die temperature 
 * @param void
 * @return the chip die temperature 
 */
uint8_t TAS2560::get_die_temp(void)
{
  uint8_t reg = readRegister8(TAS25X_DIE_TEMP_SENSOR);
  return reg;
}
uint16_t TAS2560::get_vbat(void)
{
  uint16_t reg16 = readRegister8(TAS25X_SAR_VBAT_MSB);
  reg16 = (reg16<<8) | readRegister8(TAS25X_SAR_VBAT_LSB);
  reg16 = (reg16&0X3f)>>6;
  return reg16;
}
void TAS2560::write_page(void)
{
  writeRegister8(TAS25X_PAGE,0x00); 
  writeRegister8(TAS25X_BOOK,0x00); 
}
void TAS2560::set_alarm(void)
{
  union_reg8 reg;
  reg.Bits.bit0 = 1;  /*INT_WCHLT  Sets the interrupt mask flag for the WCLK clock halt detected to     Clock Halt Modulator Clock
                        0 = Clear (not used)
                        1 = Set (used)
                      */
  reg.Bits.bit1 = 1;  /*INTM_MCHLT  Sets the interrupt mask flag for the modulator clock halt detected  
                      to    Clock Halt Word Clock
                      0 = Clear (not used)
                      1 = Set (used)
                      */
  reg.Bits.bit2 = 1;  /*INTM_CLK1 Sets the interrupt mask flag for the clock error 1 detected to    Clock Error Detection 2
                      0 = Clear (not used)
                      1 = Set (used)
                      */
  reg.Bits.bit3 = 1;  /*INTM_BRNO Sets the interrupt mask flag for the brownout detected to       Brownout
                      0 = Clear (not used)
                      1 = Set (used)
                      */
  reg.Bits.bit4 = 1;  /*INTM_OVRT Sets the interrupt mask flag for the die over-temperature         Over Temperature
                      detected to
                      0 = Clear (not used)
                      1 = Set (used)
                      */
  reg.Bits.bit5 = 1;  /*INTM_CLK2 Sets the interrupt mask flag for the clock error 2 detected to      Clock Error Detection 1
                      0 = Clear (not used)
                      1 = Set (used)
                      */
  reg.Bits.bit6 = 1;  /*INTM_AUV  Sets the interrupt mask flag for the analog under voltage
                      detected to         Speaker Over Voltage
                      0 = Clear (not used)
                      1 = Set (used)
                      */
  reg.Bits.bit7 = 1;  /*INTM_OVRI Sets the interrupt mask flag for the speaker over current
                      detected to         Speaker Over Current
                      0 = Clear (not used)
                      1 = Set (used)
                      */
  
  write_page();  
  writeRegister8(TAS25X_INT_CFG_2,0);
  // uint8_t read1  = readRegister8( TAS25X_INT_DET_1);
  // uint8_t read2 = readRegister8( TAS25X_INT_DET_2);
  writeRegister8(TAS25X_IRQ_PIN_CFG,0x01);  //0x41
  writeRegister8(TAS25X_INT_CFG_1,0x80); 
  writeRegister8(TAS25X_INT_CFG_2,reg.data);
  // uint8_t read = readRegister8(TAS25X_INT_CFG_2);
}
uint8_t TAS2560::get_alarm(void)
{
  union_reg8 reg;
  reg.data = 0;
  reg.data  = readRegister8( TAS25X_INT_DET_1);
  // uint8_t read = readRegister8( TAS25X_INT_DET_2); 
  return reg.data;
}

uint8_t TAS2560::read_INT_DET1(void)
{ 
  uint8_t read  = readRegister8( TAS25X_INT_DET_1);
  return read;
}
uint8_t TAS2560::read_INT_DET2(void)
{  
  uint8_t read = readRegister8( TAS25X_INT_DET_2);
  return read;
}

void TAS2560::set_mute(void)
{
  // Writing PWR_CTRL_1 = 0x03 (PWR_DEV = powered down, MUTE_VSNS and
  // MUTE_AUDIO set) alone did not silence the amp while clock-error
  // auto-mute was disabled: the DSP kept freewheeling its last state.
  // Zeroing SPK_CTRL's DAC gain silences the output regardless of DSP
  // state, so mute works the same at every rate. Call set_mute() before
  // stopping I2S: with the clocks stopped and the amp powered, the
  // class-D idles and hisses.
  //
  // Only cache on the first call. A second set_mute() would otherwise
  // cache gain 0 and power-state "down", and set_unmute() would then
  // restore silence.
  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, 0x00);
  if (!_muted) {
    volume = get_gain();
    // Remember whether the part was powered up with boost (PWR_DEV=01,
    // 0x40) or without (PWR_DEV=10, 0x80) so unmute restores the same
    // mode. The register tables differ: the stock table ends without boost,
    // the PPC3 8 kHz and 96 kHz tables end with boost.
    uint8_t pwr_dev = readRegister8(TAS25X_PWR_CTRL_1) & 0xC0;
    if (pwr_dev == 0x40 || pwr_dev == 0x80) _pwr_state = pwr_dev;
    _muted = true;
  }
  _write_gain(0);
  writeRegister8(TAS25X_PWR_CTRL_1, 0x03);
}
void TAS2560::set_unmute(void)
{
  // Restore the power mode the part was in before set_mute() rather
  // than always writing 0x80 (power up WITHOUT boost), which turned the
  // boost off for configurations that boot with it on.
  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, 0x00);
  writeRegister8(TAS25X_PWR_CTRL_1, _pwr_state);
  if (_muted) {
    _write_gain(volume);
    _muted = false;
  }
}

uint8_t TAS2560::set_speaker_load(uint8_t impedance)
{
  uint8_t set;
  switch(impedance)
  {
    case 4:
      set = 2;
    break;
    case 6:
      set = 1;
    break;
    case 8:
      set = 0;
    break;
    default :
      set = 0;
    break;
  }
  uint8_t reg = readRegister8(TAS25X_EDGE_ISNS_BOOST)&0xe7;
  reg = reg|(set<<3);
  writeRegister8(TAS25X_EDGE_ISNS_BOOST,reg);
  return reg;
}
uint8_t TAS2560::get_speaker_load(void)
{
  uint8_t ret = 0;
  uint8_t value = readRegister8(TAS25X_EDGE_ISNS_BOOST);

	value = (value&0x18) >> 3;

	switch (value) {
	case 0:
		ret = 8;
		break;
	case 1:
		ret = 6;
		break;
	case 2:
		ret = 4;
		break;
	default:
		break;
	}

	return ret;
}
unsigned int TAS2560::read_alarm(void)
{
	unsigned int mnErrCode = 0;
	unsigned int nDevInt1Status = 0, nDevInt2Status = 0;
	
	writeRegister8(TAS25X_INT_CFG_2, 0x00); //clear interrupt reg
	nDevInt1Status = readRegister8( TAS25X_INT_DET_1);
	nDevInt2Status = readRegister8( TAS25X_INT_DET_2);
  // Serial.printf( "IRQ critical Error : 0x%x, 0x%x\n",	nDevInt1Status, nDevInt2Status);
	if (((nDevInt1Status & 0xfc) != 0) || ((nDevInt2Status & 0xc0) != 0)) {
		/* in case of INT_OC, INT_UV, INT_OT, INT_BO, INT_CL, INT_CLK1, INT_CLK2 */	    
    
	} else {
		// Serial.printf( "IRQ status : 0x%x, 0x%x\n",	nDevInt1Status, nDevInt2Status);			
      nDevInt1Status = 0;     
		} 
    writeRegister8( TAS25X_INT_CFG_2, 0xff);
    return nDevInt1Status;
}

void TAS2560::clear_alarm(void)
{
  // writeRegister8(TAS25X_INT_CFG_2, 0x00); //clear interrupt reg
  // Keep the user's gain, channel and mute state across the reload.
  // When muted, the real gain is the cached `volume` (SPK_CTRL reads 0).
  const bool was_muted = _muted;
  const uint8_t gain = was_muted ? volume : get_gain();
  reset();
  _reload_for_current_rate();
  if (_channel_set) set_pcm_channel(_channel_mode);
  set_gain(gain);
  if (was_muted) set_mute();
}

unsigned int TAS2560::clear_interrupt(void)
{
	unsigned int mnErrCode = 0;
	unsigned int nDevInt1Status = 0, nDevInt2Status = 0;
	int nCounter = 2;

	writeRegister8(TAS25X_INT_CFG_2, 0x00); //clear interrupt reg
	nDevInt1Status = readRegister8( TAS25X_INT_DET_1);
	nDevInt2Status = readRegister8( TAS25X_INT_DET_2);

  Serial.printf( "IRQ critical Error : 0x%x, 0x%x\n",	nDevInt1Status, nDevInt2Status);

	if (((nDevInt1Status & 0xfc) != 0) || ((nDevInt2Status & 0xc0) != 0)) {
		/* in case of INT_OC, INT_UV, INT_OT, INT_BO, INT_CL, INT_CLK1, INT_CLK2 */		

		if (nDevInt1Status & 0x80) {
			mnErrCode |= ERROR_OVER_CURRENT;
			Serial.printf( "SPK over current!\n");
		} else
			mnErrCode &= ~ERROR_OVER_CURRENT;

		if (nDevInt1Status & 0x40) {
			mnErrCode |= ERROR_UNDER_VOLTAGE;
			Serial.printf( "SPK under voltage!\n");
		} else
			mnErrCode &= ~ERROR_UNDER_VOLTAGE;

		if (nDevInt1Status & 0x20) {
			mnErrCode |= ERROR_CLK_HALT;
			Serial.printf( "clk halted!\n");
		} else
			mnErrCode &= ~ERROR_CLK_HALT;

		if (nDevInt1Status & 0x10) {
			mnErrCode |= ERROR_DIE_OVERTEMP;
			Serial.printf( "die over temperature!\n");
		} else
			mnErrCode &= ~ERROR_DIE_OVERTEMP;

		if (nDevInt1Status & 0x08) {
			mnErrCode |= ERROR_BROWNOUT;
			Serial.printf( "brownout!\n");
		} else
			mnErrCode &= ~ERROR_BROWNOUT;

		if (nDevInt1Status & 0x04) {
			mnErrCode |= ERROR_CLK_LOST;
		} else
			mnErrCode &= ~ERROR_CLK_LOST;

		if (nDevInt2Status & 0x80) {
			mnErrCode |= ERROR_CLK_DET1;
			Serial.printf( "clk detection 1!\n");
		} else
			mnErrCode &= ~ERROR_CLK_DET1;

		if (nDevInt2Status & 0x40) {
			mnErrCode |= ERROR_CLK_DET2;
			Serial.printf( "clk detection 2!\n");
		} else
			mnErrCode &= ~ERROR_CLK_DET2;
		
    const bool was_muted = _muted;
    const uint8_t gain = was_muted ? volume : get_gain();
    Serial.println("config tas2560");
    reset();
    _reload_for_current_rate();
    if (_channel_set) set_pcm_channel(_channel_mode);
    set_gain(gain);
    if (was_muted) set_mute();
	} else {
		    // Serial.printf( "IRQ status : 0x%x, 0x%x\n",	nDevInt1Status, nDevInt2Status);			

      nCounter = 2;
      while (nCounter > 0) 
      {
        nDevInt1Status = readRegister8(TAS25X_STATUS_POWER);   
        if ((nDevInt1Status & 0xc0) == 0xc0)
          break;
        nCounter--;
        if (nCounter > 0) {
          /* in case check pow status just after power on TAS2560 */
          delay(10);
        }
      }

      if ((nDevInt1Status & 0xc0) != 0xc0)
      {	
			  mnErrCode |= ERROR_CLASSD_PWR;			
		  }
      else
      {
        mnErrCode &= ~ERROR_CLASSD_PWR;
      }	
      
		}

    writeRegister8( TAS25X_INT_CFG_2, 0xff);

		// if ((nDevInt1Status & 0xc0) != 0xc0) {
		// 	Serial.printf("Critical ERROR = 0x%x\n",	nDevInt1Status);
		// 	mnErrCode |= ERROR_CLASSD_PWR;				
  	// }  
	
    return mnErrCode;
}

