#include "SoftwareIIC.h"
#include "NAU85L40B.h"

IOIIC i2c_mic;

uint16_t mic_cnf[][2] = {
  {0x0000, 0x0001},  //Software reset register. Resets chip to POR state.
  {0x0001, 0x0003},  //set channels   Channel 1 analog-to-digital converter power control 0=ADC1 stage OFF 1=Enabled
  {0x0002, 0x800A},
  {0x0003, 0x804B},
  {0x0004, 0x0001},
  {0x0005, 0x0000},
  {0x0006, 0x08C0},
  {0x0007, 0x0010},
  {0x0008, 0xC000},
  {0x0009, 0x6000},
  {0x000A, 0xF13C},
  {0x000B, 0x0000},
  {0x0010, 0x0002},  //0x02=16Bits standard I2S    set sample bits
  //{0x0010, 0x000E},  //0x0e=32its standard I2S 
  {0x0011, 0x7013},
  {0x0012, 0x4800},
  {0x0013, 0x0000},
  {0x0014, 0x000F}, //ADC TX out enable for channel 1,2,3,4  1=Enable  0 = Disable
  {0x0020, 0x0000},
  {0x0021, 0x700B},
  {0x0022, 0x0022},
  {0x0023, 0x1010},
  {0x0024, 0x1010},
  {0x002D, 0x1010},
  {0x002E, 0x1010},
  {0x002F, 0x0000},
  {0x0030, 0x0000},
  {0x0031, 0x0000},
  {0x0032, 0x0000},
  {0x0033, 0x0000},
  {0x0034, 0x0000},
  {0x0035, 0x0000},
  {0x0036, 0x0000},
  {0x0037, 0x0000},
  {0x0038, 0x1F1F},
  {0x0039, 0x1F1F},
  {0x003A, 0x0002},
  {0x0040, 0x04A0}, //a0 +20dB,adc1 digital gain
  {0x0041, 0x14A0}, //a0 +20dB,adc2 digital gain
  {0x0042, 0x24A0}, //0dB,adc3 digital gain
  {0x0043, 0x34A0}, //0dB,adc4 digital gain
  {0x0044, 0x00E4}, //digital mux, 00=adc1 in, 01=adc2 in, 10=adc3 in, 11=adc4 in
  {0x0048, 0x0000},
  {0x0049, 0x0000},
  {0x004A, 0x0000},
  {0x004B, 0x0000},
  {0x004C, 0x0000},
  {0x004D, 0x0000},
  {0x004E, 0x0000},
  {0x004F, 0x0000},
  {0x0050, 0x0000},
  {0x0051, 0x0000},
  {0x0052, 0xEFFF},
  {0x0058, 0x1DF0},
  {0x005A, 0x0000},
  {0x0060, 0x0060},
  {0x0061, 0x0000},
  {0x0064, 0x0011},
  {0x0065, 0x0020},
  {0x0066, 0x000F},   //low 4 bits 1 = Power on signal ADC CH1 to CH4
  {0x0067 , 0x0D04} , //Set the output bias voltage2.5V
  // {0x0067, 0x0D07},
  {0x0068, 0x7000},
  {0x0069, 0x0088},
  {0x006A, 0x0000},
  {0x006B, 0x0101},   //0dB,channel 1/2 PGA gain
  {0x006C, 0x0101},   //0dB,channel 3/4 PGA gain
  {0x006D, 0xF000}    //Power Up Channel 4 to 1 PGA

};

NAU85L40B::NAU85L40B(uint8_t addr)
{
  i2cAddress = addr;
  #if defined(_VARIANT_RAK11200_)

  #else
  _wire = &Wire; 
  #endif
}

NAU85L40B::NAU85L40B()
{
  i2cAddress = NAU85L40B_ADDRESS;
  #if defined(_VARIANT_RAK11200_)

  #else
  _wire = &Wire; 
  #endif
}

NAU85L40B::NAU85L40B(TwoWire *w, uint8_t addr)
{
  _wire = w;
  i2cAddress = addr;
}


NAU85L40B::NAU85L40B(TwoWire *w)
{
  _wire = w;
  i2cAddress = NAU85L40B_ADDRESS;
}

bool NAU85L40B::begin(void)
{
#if defined(_VARIANT_RAK11200_)
  i2c_mic.set_pin(SDA,SCL);   
  return(i2c_mic.begin(i2cAddress));
#else
  _wire->begin();
  return true;
#endif

}

bool NAU85L40B::begin(uint8_t addr)
{
  i2cAddress = addr;
#if defined(_VARIANT_RAK11200_)
  i2c_mic.set_pin(SDA,SCL);
  return(i2c_mic.begin(i2cAddress));
#else
  _wire->begin();
  return true;
#endif

}

void NAU85L40B::writeRegister(uint16_t registerAddress, uint16_t writeData)
{
#if defined(_VARIANT_RAK11200_)
  i2c_mic.IIC_Start();  
  i2c_mic.IIC_Send_Byte( (i2cAddress<<1)&0xFE);
  i2c_mic.IIC_Wait_Ack();
//  if(i2c.IIC_Wait_Ack())
//    return NACK;    
  i2c_mic.IIC_Send_Byte( (uint8_t)((registerAddress & 0xFF00) >> 8));  
  i2c_mic.IIC_Wait_Ack();  
  i2c_mic.IIC_Send_Byte( (uint8_t)(registerAddress & 0x00FF));  
  i2c_mic.IIC_Wait_Ack();       
//  if(i2c.IIC_Wait_Ack())
//    return NACK;
  i2c_mic.IIC_Send_Byte( (uint8_t)((writeData & 0xFF00) >> 8));  
  i2c_mic.IIC_Wait_Ack();  
  i2c_mic.IIC_Send_Byte( (uint8_t)(writeData & 0x00FF));  
  i2c_mic.IIC_Wait_Ack();    
//  if(i2c.IIC_Wait_Ack())
//    return NACK;
  i2c_mic.IIC_Stop();
//  return ACK; 
#else
  _wire->beginTransmission((uint8_t)i2cAddress);
  _wire->write((uint8_t)((registerAddress & 0xFF00) >> 8));
  _wire->write((uint8_t)((uint8_t)(registerAddress & 0x00FF)));
  _wire->write((uint8_t)((writeData & 0xFF00) >> 8));
  _wire->write((uint8_t)((uint8_t)(writeData & 0x00FF)));
  _wire->endTransmission();
#endif
}

uint16_t NAU85L40B::readRegister(uint16_t registerAddress) 
{ 
#if defined(_VARIANT_RAK11200_)
  i2c_mic.IIC_Start();  
  i2c_mic.IIC_Send_Byte( (i2cAddress<<1)&0xFE );
  i2c_mic.IIC_Wait_Ack();
//  if(i2c.IIC_Wait_Ack())
//    return NACK;
  i2c_mic.IIC_Send_Byte( (uint8_t)((registerAddress & 0xFF00) >> 8));  
  i2c_mic.IIC_Wait_Ack();  
  i2c_mic.IIC_Send_Byte( (uint8_t)(registerAddress & 0x00FF));  
  i2c_mic.IIC_Wait_Ack();       
//  if(i2c.IIC_Wait_Ack())
//    return NACK;
  i2c_mic.IIC_Stop();
  delayMicroseconds(DELYA); 
  i2c_mic.IIC_Start();
  i2c_mic.IIC_Send_Byte( (i2cAddress<<1)|0x01 );
  i2c_mic.IIC_Wait_Ack();
//  if(i2c.IIC_Wait_Ack())
//    return NACK; 
  uint8_t tmp_h;
  uint8_t tmp_l;  
 
  tmp_h = i2c_mic.IIC_Read_Byte(1);
  tmp_l = i2c_mic.IIC_Read_Byte(0);       
  i2c_mic.IIC_Stop();
  return  (((uint16_t)tmp_h) << 8) |  tmp_l;
#else
  uint8_t tmp_h;
  uint8_t tmp_l;  
  _wire->beginTransmission((uint8_t)i2cAddress);
  _wire->write((uint8_t)((registerAddress & 0xFF00) >> 8));
  _wire->write((uint8_t)((uint8_t)(registerAddress & 0x00FF)));
  _wire->endTransmission();
  _wire->requestFrom((uint8_t)i2cAddress, 2);
  tmp_h = _wire->read();
  tmp_l = _wire->read();
  return  (((uint16_t)tmp_h) << 8) |  tmp_l;
#endif
}

float NAU85L40B:: fabs(float a, float b)
{
  if (a > b) return (a - b);
  else 
  return (b - a);
}

/*!
 *  @brief  set NAU85L40B ADC channel digital gain
 *  @param  volume : digital gain,max value = 36dB
 *  return	: void
 */
void NAU85L40B::setVolume(int volume)
{
  int set_value = volume;
  if(set_value<0) set_value = 0;
  set_value = set_value*8;  
  if(set_value>288) set_value = 288;  //ADC channel digital gain. Increments in -0.125dB steps, max set value +36dB  set value = 36/0.125 =288

  uint16_t ch1_volume = 0x0400;
  uint16_t ch2_volume = 0x1400;
  uint16_t ch3_volume = 0x2400;
  uint16_t ch4_volume = 0x3400;

  ch1_volume = ch1_volume + set_value;
  ch2_volume = ch2_volume + set_value;
  ch3_volume = ch3_volume + set_value;
  ch4_volume = ch4_volume + set_value;

  writeRegister(NAU85L40B_DIGITAL_GAIN_CH1,ch1_volume);
  writeRegister(NAU85L40B_DIGITAL_GAIN_CH2,ch2_volume);
  writeRegister(NAU85L40B_DIGITAL_GAIN_CH3,ch3_volume);
  writeRegister(NAU85L40B_DIGITAL_GAIN_CH4,ch4_volume);

}

/*!
 *  @brief  Calculate the NAU85L40B register FLL frequency division coefficient 
 *  @param  freq : I2S FS frequency
 *  @param  *cnf : register divider value
 *  return	: void
 */
void NAU85L40B::set_mic(int freq, cnf_fll_t *cnf)
{
  uint8_t src_reg[10][2] = {
    {1, 0},   //Pass through
    {2, 2},   //Divide by 2
    {3, 7},   //Divide by 3
    {4, 3},   //Divide by 4
    {6, 10},  //Divide by 6
    {8, 4},   //Divide by 8
    {12, 11}, //Divide by 12
    {16, 5},  //Divide by 16
    {24, 12}, //Divide by 24
    {32, 6}  //Divide by 32
    //    {Invert,1}
    //    {Invert,9}
  };

  /*
    //Set the reference frequency to FS clock
    //Recommended FDCO clock at 90Mhz-100Mhz
    FDCO = FREF * FLL_INTEGER.FLL_FRAC ÷ FLL4.FLL_CLK_REF_DIV
    MCLK = (FDCO * MCLK_SRC)/2
  */
  uint16_t fll_integer = 0;
  float fll_frac = 0;
  uint8_t fll_ref_div = 1; //default set 1 

  uint32_t fdco = 0;
  uint32_t fdco_max = 100000000;  //FDCO MAX 100Mhz
  uint32_t fdco_min = 90000000;  //FDCO Min 90Mhz
  float mck_src_max = 0;
  float mck_src_min = 0;
  uint8_t mck_src = 0;
  uint8_t mck_src_reg = 0;
  mck_src_max = (float)fdco_max / (freq * 512);
  mck_src_min = (float)fdco_min / (freq * 512);
  float delt_min = 32;
  //  Serial.printf("setFreq:%d  mckSrcMax:%3.2f  mckSrcMin:%3.2f\r\n",freq,mck_src_max,mck_src_min);
  for (int i = 0; i < (sizeof(src_reg) / sizeof(src_reg[0])); i++)
  {
    //    Serial.println(src_reg[i][0]);
    float ftemp = fabs(mck_src_max, src_reg[i][0]);
    if (delt_min > ftemp)
    {
      delt_min = ftemp;
      mck_src = src_reg[i][0];
      mck_src_reg = src_reg[i][1];
    }
    ftemp = fabs(mck_src_min, src_reg[i][0]);
    if (delt_min > ftemp)
    {
      delt_min = ftemp;
      mck_src = src_reg[i][0];
      mck_src_reg = src_reg[i][1];
    }
  }
  //  Serial.printf("mck_src:%d\r\n",mck_src);
  fdco = freq * 512 * mck_src;
    // Serial.printf("fdco:%d\r\n",fdco);

  float f_reg = (float)fdco / freq;
    // Serial.printf("f_reg:%f\r\n",f_reg); 

  uint16_t infreq = freq / 1000;
  if (infreq >= 512)
  {
    cnf->fll_ratio = 1;
  }
  else if (infreq >= 256)
  {
    cnf->fll_ratio = 2;
  }
  else if (infreq >= 128)
  {
    cnf->fll_ratio = 4;
  }
  else if (infreq >= 64)
  {
    cnf->fll_ratio = 8;
  }
  else if (infreq >= 32)
  {
    cnf->fll_ratio = 16;
  }
  else if (infreq >= 8)
  {
    cnf->fll_ratio = 32;
  }
  else if (infreq >= 4)
  {
    cnf->fll_ratio = 64;
  }

  //  fll_ref_div = 8;
  float fnum = (float)(f_reg / fll_ref_div / cnf->fll_ratio);
    // Serial.printf("fnum:%f\r\n",fnum);
  fll_integer = (uint16_t)(fnum * 10) / 10;
  fll_frac = fnum - fll_integer;
    // Serial.printf("fll_integer:%d  fll_frac:%f\r\n",fll_integer,fll_frac);

  cnf->fll_integer = fll_integer & 0x3FF;
  cnf->fll_frac = fll_frac;
  cnf->mclk_src = mck_src_reg & 0x0F;
  cnf->fll_clk_ref_div = 0; //fll_ref_div = 1 config Bits = 0
  
  // Serial.printf("MCLK_SRC:%d  REG:%02X\r\n",cnf->mclk_src,mck_src_reg);
  // Serial.printf("FLL_INTEGER:%d  REG:%02X\r\n",cnf->fll_integer,cnf->fll_integer);
  // Serial.printf("FLL_FRAC:%.3f  REG:%04X\r\n",cnf->fll_frac,cnf->fll_frac*65536);
  // Serial.printf("FLL_CLK_REF_DIV:%d  REG:%02X\r\n",fll_ref_div,cnf->fll_clk_ref_div);
  // Serial.printf("FLL_RATIO:%d\r\n",cnf->fll_ratio);
}
/*!
 *  @brief  Register Initial Configuration
 *  @param  sampleRate : I2S sample rate
 *  @param  channels : I2S channels
 *  @param  sampleBit : I2S per sample bits
 *  return	: void
 */
void NAU85L40B::config(int sampleRate, uint16_t channels,uint16_t sampleBit)
{
  uint16_t i = 0;

  uint16_t reg1 = (uint16_t)channels;    //set channel register address 0x0001
  uint16_t reg10 = (uint16_t)sampleBit;   //set sample bits register address 0x0010
  
  uint16_t reg3 = 0x804B;
  uint16_t reg4 = 0X0001;
  uint16_t reg5 = 0x0000;
  uint16_t reg6 = 0x08C0;
  uint16_t reg7 = 0x0010;  
  uint16_t reg3A = 0x0002;

  cnf_fll_t cnfreg;
  set_mic(sampleRate, &cnfreg);

  uint16_t  fll_integer;
  float  fll_frac;
  uint8_t mclk_src;
  uint8_t fll_clk_ref_div;
 
  reg3 = 0x8040 | cnfreg.mclk_src;
  reg4 = cnfreg.fll_ratio&0x007F;
  reg5 = (uint16_t)(cnfreg.fll_frac * 65536);  
  reg6 = cnfreg.fll_integer|0x0C00;
  reg7 = ((cnfreg.fll_clk_ref_div & 0x03) << 10) | 0x10;
  reg3A = 0X0002;  //CLK_ADC_SRC=4  ADC_CLK = MCLK/CLK_ADC_SRC = 256FS/4=64FS, OSR = CLK_ADC/FS=64
 
  // Serial.printf("reg3:%04X\r\n",reg3);
  // Serial.printf("reg4:%04X\r\n",reg4);
  // Serial.printf("reg5:%04X\r\n",reg5);
  // Serial.printf("reg6:%04X\r\n",reg6);
  // Serial.printf("reg7:%04X\r\n",reg7);
  // Serial.printf("reg3A:%04X\r\n",reg3A);

  // Serial.println("config NAU85L40B ");
  for (i = 0; i < sizeof(mic_cnf) / sizeof(mic_cnf[0]); i++)
  {               
    switch(mic_cnf[i][0])
    {
      case 0x0001:  //NAU85L40B_POWER_MANAGEMENT:  //0x0001  set channel
        mic_cnf[i][1] = reg1;
      break;
      case 0x0003:  //NAU85L40B_CLOCK_SRRC:        //0x0003
        mic_cnf[i][1] = reg3;
      break;  
      case 0x0004:  //NAU85L40B_FLL1:              //0x0004
        mic_cnf[i][1] = reg4;
      break;
      case 0x0005:  //NAU85L40B_FLL2:              //0x0005
        mic_cnf[i][1] = reg5;
      break;
      case 0x0006:  //NAU85L40B_FLL3:              //0x0006
        mic_cnf[i][1] = reg6;
      break;
      case 0x0007:  //NAU85L40B_FLL4:              //0x0007
        mic_cnf[i][1] = reg7;
      break;
      case 0x0010:  //NAU85L40B_PCM_CTRL0:         //0x0010  set sample bits 
        mic_cnf[i][1] = reg10;
      break;
      case 0x003A:  //NAU85L40B_ADC_SIMPLE_RATE:   //0x003A
        mic_cnf[i][1] = reg3A;
      break;
      default :break;
    }
    
    // Serial.printf("%04X\t%04X\r\n", mic_cnf[i][0], mic_cnf[i][1]);
    writeRegister(mic_cnf[i][0], mic_cnf[i][1]);
  }

  // Serial.println("read NAU85L40B ");
  // for (i = 0; i < sizeof(mic_cnf) / sizeof(mic_cnf[0]); i++)
  // {
  //   uint16_t temp = readRegister(mic_cnf[i][0]);

  //   if (temp != mic_cnf[i][1])
  //   {
  //     Serial.printf("%04X\t%04X\r\n", mic_cnf[i][0], mic_cnf[i][1]);
  //   }
  // }

}
