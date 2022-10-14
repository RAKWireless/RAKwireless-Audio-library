#ifndef __TAS2560REG_H__
#define __TAS2560REG_H__

#include <arduino.h>
#include <Wire.h>
// #include "tas2560.h"

#define CFG_META_SWITCH (255)
#define CFG_META_DELAY  (254)
#define CFG_META_BURST  (253)

#define TAS25X_PAGE            0x00  //Page Select PAGE (book=0x00 page=0x00 address=0x00) [reset=0h]
#define TAS25X_RESET           0x01  //set 1 to reset chip. Software Reset RESET (book=0x00 page=0x00 address=0x01) [reset=0h]
#define TAS25X_MODE            0x02  //Mode Control MODE (book=0x00 page=0x00 address=0x02) [reset=1h]
#define TAS25X_SPK_CTRL        0x04  //Speaker Control SPK_CTRL (book=0x00 page=0x00 address=0x04) [reset=5Fh]
#define TAS25X_PWR_CTRL_2      0x05  //Power Up Control 2 PWR_CTRL_2 (book=0x00 page=0x00 address=0x05) [reset=0h]
#define TAS25X_PWR_CTRL_1      0x07  //Power Up Control 1 PWR_CTRL_1 (book=0x00 page=0x00 address=0x07) [reset=0h]
#define TAS25X_RAMP_CTRL       0x08  //Class RAMP_CTRL (book=0x00 page=0x00 address=0x08) [reset=1h]
#define TAS25X_EDGE_ISNS_BOOST 0x09  //Edge Rate, Isense Scale, Boost limit EDGE_ISNS_BOOST (book=0x00 page=0x00 address=0x09) [reset=83h]
#define TAS25X_PLL_CLKIN       0x0F  //PLL Clock Input Control PLL_CLKIN (book=0x00 page=0x00 address=0x0F)[reset=41h]
#define TAS25X_PLL_JVAL        0x10  //PLL J Multiplier Control PLL_JVAL (book=0x00 page=0x00 address=0x10) [reset=4h]
#define TAS25X_PLL_DVAL_1      0x11  //PLL Fractional Multiplier D Val MSB PLL_DVAL_1 (book=0x00 page=0x00 address=0x11) [reset=0h]
#define TAS25X_PLL_DVAL_2      0x12  //PLL Fractional Multiplier D Val LSB PLL_DVAL_2 (book=0x00 page=0x00 address=0x12) [reset=0h]
#define TAS25X_ASI_FORMAT      0x14  //ASI Mode Control ASI_FORMAT (book=0x00 page=0x00 address=0x14) [reset=2h]
#define TAS25X_ASI_CHANNEL     0x15  //ASI Channel Control ASI_CHANNEL (book=0x00 page=0x00 address=0x15) [reset=0h]
#define TAS25X_ASI_OFFSET_1    0x16  //ASI Offset ASI_OFFSET_1 (book=0x00 page=0x00 address=0x16) [reset=0h]
#define TAS25X_ASI_OFFSET_2    0x17  //ASI Offset Second Slot ASI_OFFSET_2 (book=0x00 page=0x00 address=0x17) [reset=0h]
#define TAS25X_ASI_CFG_1       0x18  //ASI Configuration ASI_CFG_1 (book=0x00 page=0x00 address=0x18) [reset=0h]
#define TAS25X_ASI_DIV_SRC     0x19  //ASI BDIV Clock Input ASI_DIV_SRC (book=0x00 page=0x00 address=0x19) [reset=0h]
#define TAS25X_ASI_BDIV        0x1A  //ASI BDIV Configuration ASI_BDIV (book=0x00 page=0x00 address=0x1A) [reset=1h]
#define TAS25X_ASI_WDIV        0x1B  //ASI WDIV Configuration ASI_WDIV (book=0x00 page=0x00 address=0x1B) [reset=40h]
#define TAS25X_PDM_CFG         0x1C  //PDM Configuration PDM_CFG (book=0x00 page=0x00 address=0x1C) [reset=0h]
#define TAS25X_PDM_DIV         0x1D  //PDM Divider Configuration PDM_DIV (book=0x00 page=0x00 address=0x1D) [reset=8h]
#define TAS25X_DSD_DIV         0x1E  //DSD Divider Configuration DSD_DIV (book=0x00 page=0x00 address=0x1E) [reset=8h]
#define TAS25X_CLK_ERR_1       0x21  //Clock Error and DSP memory Reload CLK_ERR_1 (book=0x00 page=0x00 address=0x21) [reset=3h]
#define TAS25X_CLK_ERR_2       0x22  //Clock Error Configuration CLK_ERR_2 (book=0x00 page=0x00 address=0x22) [reset=3Fh]
#define TAS25X_IRQ_PIN_CFG     0x23  //Interrupt Pin Configuration IRQ_PIN_CFG (book=0x00 page=0x00 address=0x23) [reset=21h]
#define TAS25X_INT_CFG_1       0x24  //Interrupt Configuration 1 INT_CFG_1 (book=0x00 page=0x00 address=0x24) [reset=0h]
#define TAS25X_INT_CFG_2       0x25  //Interrupt Configuration 2 INT_CFG_2 (book=0x00 page=0x00 address=0x25) [reset=0h]
#define TAS25X_INT_DET_1       0x26  //Interrupt Detected 1 INT_DET_1 (book=0x00 page=0x00 address=0x26) [reset=0h]
#define TAS25X_INT_DET_2       0x27  //Interrupt Detected 2 INT_DET_2 (book=0x00 page=0x00 address=0x27) [reset=0h]
#define TAS25X_STATUS_POWER     0x2A  //Status Block Power STATUS_POWER (book=0x00 page=0x00 address=0x2A) [reset=0h]   This register indicated the operational status of various internal blocks
#define TAS25X_SAR_VBAT_MSB     0x2D  //SAR VBAT Measurement MSB SAR_VBAT_MSB (book=0x00 page=0x00 address=0x2D) [reset=C0h]
#define TAS25X_SAR_VBAT_LSB     0x2E  //SAR VBAT Measurement LSB SAR_VBAT_LSB (book=0x00 page=0x00 address=0x2E) [reset=0h]
#define TAS25X_DIE_TEMP_SENSOR  0x31  //Die Temperature Sensor DIE_TEMP_SENSOR (book=0x00 page=0x00 address=0x31) [reset=0h]
#define TAS25X_LOW_PWR_MODE     0x35  //Low Power Configuration LOW_PWR_MODE (book=0x00 page=0x00 address=0x35) [reset=0h]
#define TAS25X_PCM_RATE         0x36  //PCM Sample Rate PCM_RATE (book=0x00 page=0x00 address=0x36) [reset=32h]
#define TAS25X_CLOCK_ERR_CFG_1  0x4F  //Clock Error Configuration 1 CLOCK_ERR_CFG_1 (book=0x00 page=0x00 address=0x4F) [reset=0h]
#define TAS25X_CLOCK_ERR_CFG_2  0x50  //Clock Error Configuration 2 CLOCK_ERR_CFG_2 (book=0x00 page=0x00 address=0x50) [reset=11h]
#define TAS25X_PROTECTION_CFG_1 0x58  //Class PROTECTION_CFG_1 (book=0x00 page=0x00 address=0x58) [reset=3h] 0x7E CRC_CHECKSUM Checksum CRC_CHECKSUM (book=0x00 page=0x00 address=0x7E) [reset=0h]
#define TAS25X_BOOK             0x7F  //Book Selection BOOK (book=0x00 page=0x00 address=0x7F) [reset=0h

//----------------------------------------------------------------------------------------------------------------
/*ASI PCM Mode ASI_FORMAT[4:2] (ASI_MODE) ASI Function Mode */

#define I2S_MODE        0x0    // I2S Mode(default)         0b00000 
#define DSP_MODE        0x04   // DSP Mode                  0b00100
#define RJF_MODE        0x08   // Right Justified Mode      0b01000
#define LJF_MODE        0x0c   // Left-Justified Mode       0b01100
#define PCM_MODE        0x10   // PCM Mode Mono             0b10000
#define DSP_TS_MODE     0x14   // DSP Time Slot Mode        0b10100

/*ASI PCM Input Word Length  ASI_FORMAT[1:0] (ASI_LENGTH) Word Length */
#define PCM_LEN16       0x0     //16 bits             0b00
#define PCM_LEN20       0x01    //20 bits             0b01
#define PCM_LEN24       0x02    //24 bits (default)   0b10
#define PCM_LEN32       0x03    //32 bits             0b11

#define	ERROR_NONE			   0x00000000
#define	ERROR_PLL_ABSENT	   0x00000001
#define	ERROR_DEVA_I2C_COMM	0x00000002
#define	ERROR_DEVB_I2C_COMM	0x00000004
#define	ERROR_PRAM_CRCCHK	   0x00000008
#define	ERROR_YRAM_CRCCHK	   0x00000010
#define	ERROR_CLK_DET2		   0x00000020
#define	ERROR_CLK_DET1		   0x00000040
#define	ERROR_CLK_LOST		   0x00000080
#define	ERROR_BROWNOUT		   0x00000100
#define	ERROR_DIE_OVERTEMP	0x00000200
#define	ERROR_CLK_HALT		   0x00000400
#define	ERROR_UNDER_VOLTAGE	0x00000800
#define	ERROR_OVER_CURRENT	0x00001000
#define	ERROR_CLASSD_PWR	   0x00002000
#define	ERROR_FAILSAFE		   0x40000000

#define	TAS2560_INTM_OVRI      0x80
#define	TAS2560_INTM_AUV       0x40
#define	TAS2560_INTM_CLK2      0x20
#define	TAS2560_INTM_OVRT      0x10
#define	TAS2560_INTM_BRNO      0x08
#define	TAS2560_INTM_CLK1      0x04
#define	TAS2560_INTM_MCHLT     0x02
#define	TAS2560_INT_WCHLT      0x01

/*ASI ASI_CHAN_MODE[1:0] Configures the ASI input stereo channel mode.   */
typedef enum {
   LeftMode  = 0,        //0 = Left Channel
   RightMode = 1,        //1 = Right Channel
   LeftRightMode = 2,    //2 = (Left + Right) / 2
   MonoPCMMode = 3       //3 = monoPCM
}ASI_channel;

typedef unsigned char cfg_u8;

typedef union {
   struct {
    cfg_u8 command;
    cfg_u8 param;
 }; 
} cfg_reg;

extern cfg_reg registers[205];

typedef union 
{   
   struct //bit
   {
      uint8_t bit0   :1;
      uint8_t bit1   :1;
      uint8_t bit2   :1;
      uint8_t bit3   :1;
      uint8_t bit4   :1;
      uint8_t bit5   :1;
      uint8_t bit6   :1;
      uint8_t bit7   :1;
   }Bits;

   uint8_t data;   
}union_reg8;



#endif /* __TAS2560REG_H__ */
