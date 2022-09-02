#ifndef _NAU85L40B_H_
#define _NAU85L40B_H_

#include "Arduino.h"
#include <Wire.h>
#include "audio.h"


typedef enum _MIC_Channels_t  //NAU85L40B Channels
{
  MIC_CHANNEL1 = 1,
  MIC_CHANNEL2 = 2,
  MIC_CHANNEL3 = 4,
  MIC_CHANNEL4 = 8
}MIC_Channels_t;

typedef enum _MIC_SampleBits_t //NAU85L40B Sample Bits
{
  I2S_SAMPLE_16BIT = 2,
  I2S_SAMPLE_20BIT = 6,
  I2S_SAMPLE_24BIT = 10,
  I2S_SAMPLE_32BIT = 14

}MIC_SampleBits_t;

typedef struct cnf_fll_s{
  uint16_t  fll_integer;
  float  fll_frac;
  uint8_t mclk_src;
  uint16_t fll_ratio;
  uint8_t fll_clk_ref_div;
} cnf_fll_t;

#define NAU85L40B_ADDRESS 0x1C

// 16bit address MSB
#define NAU85L40B_RESET                      0x0000
#define NAU85L40B_POWER_MANAGEMENT           0x0001
#define NAU85L40B_CLOCK_CTRL                 0x0002
#define NAU85L40B_CLOCK_SRRC                 0x0003
#define NAU85L40B_FLL1                       0x0004
#define NAU85L40B_FLL2                       0x0005
#define NAU85L40B_FLL3                       0x0006
#define NAU85L40B_FLL4                       0x0007
#define NAU85L40B_FLL5                       0x0008
#define NAU85L40B_FLL6                       0x0009
#define NAU85L40B_FLL_VCO_RSV                0x000A
#define NAU85L40B_PCM_CTRL0                  0x0010
#define NAU85L40B_PCM_CTRL1                  0x0011
#define NAU85L40B_PCM_CTRL2                  0x0012
#define NAU85L40B_PCM_CTRL3                  0x0013
#define NAU85L40B_PCM_CTRL4                  0x0014
#define NAU85L40B_ALC_CONTROL_1              0x0020
#define NAU85L40B_ALC_CONTROL_2              0x0021
#define NAU85L40B_ALC_CONTROL_3              0x0022
#define NAU85L40B_ALC_CONTROL_4              0x0023
#define NAU85L40B_ALC_CONTROL_5              0x0024
#define NAU85L40B_ALC_GAIN_CH12              0x002D
#define NAU85L40B_ALC_GAIN_CH34              0x002E
#define NAU85L40B_ALC_STATUS                 0x002F
#define NAU85L40B_NOTCH_FIL1_CH1             0x0030
#define NAU85L40B_NOTCH_FIL2_CH1             0x0031
#define NAU85L40B_NOTCH_FIL1_CH2             0x0032
#define NAU85L40B_NOTCH_FIL2_CH2             0x0033
#define NAU85L40B_NOTCH_FIL1_CH3             0x0034
#define NAU85L40B_NOTCH_FIL2_CH3             0x0035
#define NAU85L40B_NOTCH_FIL1_CH4             0x0036
#define NAU85L40B_NOTCH_FIL2_CH4             0x0037
#define NAU85L40B_HPF_FILTER_CH12            0x0038
#define NAU85L40B_HPF_FILTER_CH34            0x0039
#define NAU85L40B_ADC_SIMPLE_RATE            0x003A
#define NAU85L40B_DIGITAL_GAIN_CH1           0x0040
#define NAU85L40B_DIGITAL_GAIN_CH2           0x0041
#define NAU85L40B_DIGITAL_GAIN_CH3           0x0042
#define NAU85L40B_DIGITAL_GAIN_CH4           0x0043
#define NAU85L40B_DIGITAL_MUX                0x0044
#define NAU85L40B_P2P_CH1                    0x0048
#define NAU85L40B_P2P_CH2                    0x0049
#define NAU85L40B_P2P_CH3                    0x004A
#define NAU85L40B_P2P_CH4                    0x004B
#define NAU85L40B_PEAK_CH1                   0x004C
#define NAU85L40B_PEAK_CH2                   0x004D
#define NAU85L40B_PEAK_CH3                   0x004E
#define NAU85L40B_PEAK_CH4                   0x004F
#define NAU85L40B_GPIO_CTRL                  0x0050
#define NAU85L40B_MISC_CTRL                  0x0051
#define NAU85L40B_I2C_CTRL                   0x0052
#define NAU85L40B_I2C_DEVICE_ID              0x0058
#define NAU85L40B_RST                        0x005A
#define NAU85L40B_VMID_CTRL                  0x0060
#define NAU85L40B_MUTE                       0x0061
#define NAU85L40B_ANALOG_ADC1                0x0064
#define NAU85L40B_ANALOG_ADC2                0x0065
#define NAU85L40B_ANALOG_PWR                 0x0066
#define NAU85L40B_MIC_BIAS                   0x0067
#define NAU85L40B_REFERENCE                  0x0068
#define NAU85L40B_FEPGA1                     0x0069
#define NAU85L40B_FEPGA2                     0x006A
#define NAU85L40B_FEPGA3                     0x006B
#define NAU85L40B_FEPGA4                     0x006C
#define NAU85L40B_PWR                        0x0069


class NAU85L40B
{
public:
   NAU85L40B();
   NAU85L40B(TwoWire *w);
   NAU85L40B(uint8_t addr);
   NAU85L40B(TwoWire *w, uint8_t addr);
   bool begin(void);
   bool begin(uint8_t addr);
   void setVolume(int volume);
   void config(int sampleRate, uint16_t channels,uint16_t sampleBit);
   uint16_t readRegister(uint16_t registerAddress);
   void writeRegister(uint16_t registerAddress, uint16_t writeData); 

   private:

   uint8_t i2cAddress;
   uint16_t bit_depth = 16;
   TwoWire *_wire;
   void set_mic(int freq, cnf_fll_t *cnf);
   float fabs(float a, float b);
};

#endif
