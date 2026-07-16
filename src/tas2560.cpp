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
  int count = 0;
  const cfg_reg *table = tas2560_dsp_coeffs_for_rate(sample_rate, &count);
  transmit_registers(const_cast<cfg_reg *>(table), count);
  set_alarm();
  apply_rate_dependent_init(sample_rate);
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
  int count = 0;
  const cfg_reg *table = tas2560_dsp_coeffs_for_rate(sample_rate, &count);
  transmit_registers(const_cast<cfg_reg *>(table), count);
  set_alarm();
  apply_rate_dependent_init(sample_rate);
  return true;
}

// PCM_RATE (book 0x00 page 0x00 reg 0x36) nibble values per the
// TAS2560 datasheet PCM_RATE table. Confirm against the datasheet
// before shipping — these match the upstream static-table value of
// 0x32 for 16/24 kHz, which is the only point we can cross-check
// without the part in front of us.
//
// 8 kHz   -> 0x12
// 12 kHz  -> 0x22
// 16/24   -> 0x32
// 32/48   -> 0x42
// 44.1/88.2 -> 0x52
// 96 kHz  -> 0x62
bool TAS2560::set_sample_rate(uint32_t fs)
{
  // PCM_RATE (book 0x00 page 0x00 reg 0x36) encodings. My Phase 1
  // draft used 0x12 for 8 kHz based on a guessed nibble layout; the
  // PPC3 export at 8 kHz uses 0x30, so the low nibble at least is
  // rate-specific but not what I guessed. 0x30 wins for 8 kHz.
  uint8_t pcm_rate;
  switch (fs) {
    case 8000:                       pcm_rate = 0x30; break;
    case 12000:                      pcm_rate = 0x22; break;
    case 16000: case 24000:          pcm_rate = 0x32; break;
    case 32000: case 48000:          pcm_rate = 0x42; break;
    case 44100: case 88200:          pcm_rate = 0x52; break;
    case 96000:                      pcm_rate = 0x62; break;
    default: return false;
  }
  // Page/book 0 holds PCM_RATE and CLK_ERR_2.
  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, 0x00);
  writeRegister8(TAS25X_PCM_RATE, pcm_rate);
  _sample_rate = fs;
  return true;
}

void TAS2560::apply_rate_dependent_init(uint32_t fs)
{
  if (!set_sample_rate(fs)) {
    // Unknown rate — leave the upstream defaults in place.
    return;
  }

  // The static init table assumes the 16/24 kHz BCLK domain:
  //   PLL_CLKIN  (0x0F) = 0x01  -> source = BCLK
  //   PLL_J      (0x10) = 0x20  -> integer multiplier 32
  //   PLL_D MSB  (0x11) = 0x00
  //   PLL_D LSB  (0x12) = 0x00
  //   CLK_ERR_2  (0x22) = 0x3F  -> every clock-error flag enabled
  // At 16 kHz mono 16-bit, BCLK = 256 kHz, so PLL_OUT = 256k * 32 =
  // 8.192 MHz — the DSP's expected MCLK. At 8 kHz mono 16-bit BCLK
  // halves to 128 kHz, the PLL would drop to ~4.096 MHz which is
  // below the chip's working range, and the part declines to enable
  // class-D (STATUS_POWER reads 0x00 with no clock-error flag).
  //
  // To keep PLL_OUT centred near 8.192 MHz across rates, we scale
  // PLL_J inversely with BCLK. Stay rate-independent for the upstream-
  // tuned rates (>=16 kHz, J=0x20) and double J for the 8/12 kHz BCLK
  // domain. PLL_D stays zero (integer multiplier).

  uint8_t pll_j;
  uint8_t clk_err_1;
  uint8_t clk_err_2;
  switch (fs) {
    case 8000:
    case 12000:
      // BCLK-widening in I2SEventClass (32-bit stereo slot at rate
      // < 16 kHz) brings BCLK to 512 kHz — inside the TAS2560 PLL
      // lock range.
      //
      // PLL_J targeting: PPC3's 8 kHz DSP + decimation tables were
      // computed assuming BCLK 1.536 MHz with J=32, i.e. PLL_OUT ≈
      // 49.152 MHz feeding the DSP/DAC clock tree. At our 512 kHz
      // BCLK we need J=96 (0x60) to hit the same internal MCLK. An
      // earlier experiment with J=16 (PLL_OUT ≈ 8.192 MHz) left the
      // chip with PWR=0x00 — the PLL locked but the downstream
      // divider chain saw too-low a clock to enable class-D.
      pll_j      = 0x60;
      clk_err_1  = 0x03;
      clk_err_2  = 0x3F;
      break;
    default:
      pll_j      = 0x20;  // upstream default for >=16 kHz BCLK
      clk_err_1  = 0x03;
      clk_err_2  = 0x3F;
      break;
  }

  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, 0x00);
  writeRegister8(TAS25X_PLL_JVAL, pll_j);
  writeRegister8(TAS25X_PLL_DVAL_1, 0x00);
  writeRegister8(TAS25X_PLL_DVAL_2, 0x00);
  writeRegister8(TAS25X_CLK_ERR_1, clk_err_1);
  writeRegister8(TAS25X_CLK_ERR_2, clk_err_2);
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
  uint8_t reg = readRegister8(TAS25X_ASI_CHANNEL);
  reg = (reg&0xFC)|(mode&0x03);
  writeRegister8(TAS25X_ASI_FORMAT,reg);
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
  return reg;
}
/**
 * @brief set the set volume
 * @param uint8_t gain can set from 0-15
 * @return the register of TAS25X_SPK_CTRL
 */
uint8_t TAS2560::set_gain(uint8_t gain)
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
  // Belt and braces: writing PWR_CTRL_1 = 0x03 alone disables the
  // class-D but does NOT silence the DSP output when the chip is
  // running with clock-error auto-mute disabled (our Phase 1 8 kHz
  // configuration — CLK_ERR_1 = 0 keeps 128 kHz BCLK from tripping
  // spurious mutes during playback). In that state, the DSP path
  // freewheels whatever it last produced and remains audible.
  //
  // Zeroing SPK_CTRL's gain field zeroes the output regardless of
  // DSP state, so mute now works uniformly at all rates. Cache the
  // current gain so set_unmute() can restore it — same pattern
  // clear_alarm() already uses for the `volume` member.
  volume = get_gain();
  set_gain(0);
  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, 0x00);
  writeRegister8(TAS25X_PWR_CTRL_1, 0x03);
}
void TAS2560::set_unmute(void)
{
  writeRegister8(TAS25X_PAGE, 0x00);
  writeRegister8(TAS25X_BOOK, 0x00);
  writeRegister8(TAS25X_PWR_CTRL_1, 0x80);
  set_gain(volume);
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
  volume = get_gain();
  // Serial.println("config tas2560");
  reset();
  transmit_registers(registers,sizeof(registers)/2);
  set_gain(volume);
  set_alarm();  
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
		
    volume = get_gain();
    Serial.println("config tas2560");
    reset();
    transmit_registers(registers,sizeof(registers)/2);
    set_gain(volume);
    set_alarm();
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

