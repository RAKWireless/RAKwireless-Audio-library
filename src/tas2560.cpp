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
 writeRegister8(TAS25X_PAGE, 0x00 );       //---------------
 writeRegister8(TAS25X_BOOK, 0x00 );
 writeRegister8(TAS25X_PWR_CTRL_1, 0x03 );

}
void TAS2560::set_unmute(void)
{
 writeRegister8(TAS25X_PAGE, 0x00 );       //---------------
 writeRegister8(TAS25X_BOOK, 0x00 );
 writeRegister8(TAS25X_PWR_CTRL_1, 0x80 );

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

