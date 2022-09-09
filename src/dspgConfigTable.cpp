
#include "dspg.h"

// COMMAND D10L_config_table[COMMAND_TABLE_LENGTH]=
// {
// 	//0x029 ,0x1048	, VAL_SHORT,   	//Set SPI_D2_TDM1_D4_GPIO_4_5_6_7_D10_GPIO_2_3_4_5 | UART_DEBUG_MODE_0 | UART_D2_0_UART_TDI_RTCK_D4_1_GPIO_14_15_D10_GPIO_12_13
// 	0x012 ,0xc01e	, VAL_SHORT,	//set Events Indication Enable Configuration
// 	0x015 ,0x9a99	, VAL_SHORT,	//FW ready - GPIO26; INT/EVENT - GPIO25, Active high
// 	0x01b ,0x0020	, VAL_SHORT,	//Set MCLK 32.768Khz
// 	0x009 ,0x07d0	, VAL_SHORT,	//AUDIO BUFFER SIZE 1 Seconds
// 	0x022 ,0x1001	, VAL_SHORT,	//Set Wake Up Polarity high
// 	0x01a ,0x1003	, VAL_SHORT,    //HPF enable/disable 
// 	0x033 ,0x0186	, VAL_SHORT,	//Memory Configuration
// 	0x023 ,0x0200	, VAL_SHORT,    //Mic config
// 	0x010 ,0x1013000, VAL_LONG, 	//TL3 frequency = 110.592 MHz
// 	0x00c ,0x83e8	, VAL_SHORT,	//Set Uart1 baudrate 1Mhz
// 	0x01a ,0x0001	, VAL_SHORT,	//HPF enable/disable
// 	0x024 ,0x0261	, VAL_SHORT,	//RISING_EDGE, DDF0_DM0, 0dB
// 	0x024 ,0x0200	, VAL_SHORT,
// 	0x005 ,0x0000	, VAL_SHORT,
// 	0x01f ,0x0000	, VAL_SHORT,
// };

COMMAND D10L_config_table[]=
{
	//0x029 ,0x1048	, VAL_SHORT,   	//Set D10_SPI0_GPIO_27_28_29_30 | UART_DEBUG_MODE_0 | UART_D2_0_UART_TDI_RTCK_D4_1_GPIO_14_15_D10_GPIO_12_13	do config it, otherwise GP13 would not output log
	0x012 ,0xc01e	, VAL_SHORT,
	0x015 ,0x9a99	, VAL_SHORT,	//FW ready - GPIO26; INT/EVENT - GPIO25, Active high
	0x01b ,0x0020	, VAL_SHORT,	//Set MCLK 32.768Khz
	0x009 ,0x03e8	, VAL_SHORT,	//AUDIO BUFFER SIZE half Second
	0x022 ,0x1001	, VAL_SHORT,	//Set Wake Up Polarity high
	0x033 ,0x0186	, VAL_SHORT,	
	0x023 ,0x0200	, VAL_SHORT,
	0x010 ,0x1013000, VAL_LONG, 	//TL3 frequency = 110.592 MHz
#ifdef ASRP_RECORDING
	0x00c ,0x8bb8	, VAL_SHORT,	//Set Uart1 baudrate 3Mhz
#else
	0x00c ,0x83e8	, VAL_SHORT,	//Set Uart1 baudrate 1Mhz
#endif
	0x01a ,0x0001	, VAL_SHORT,
	0x024 ,0x0261	, VAL_SHORT,	//RISING_EDGE, DDF0_DM0, 0dB
	0x024 ,0x0200	, VAL_SHORT,
	0x005 ,0x0000	, VAL_SHORT,
#if USE_HIGHER_GPIO_AT_3V3
#if TDM_CLK_SHARED
	0x01f, 0x0004	 , VAL_SHORT,	//MUSIC_IN_TDM0 | TDM1_3V3
#else
	0x01f, 0x0024	 , VAL_SHORT,	//MUSIC_IN_TDM0 | TDM1_3V3 | TDM1_IOS_NOT_SHARED
#endif
#else
#if TDM_CLK_SHARED
	0x01f, 0x0004	 , VAL_SHORT,	//MUSIC_IN_TDM0 | TDM1_3V3
#else
	0x01f, 0x0024	 , VAL_SHORT,	//MUSIC_IN_TDM0 | TDM1_3V3 | TDM1_IOS_NOT_SHARED
#endif
#endif

};

// COMMAND RT_CMD_D10L_evb_bargein_1mic_enter[]=
// {
// #ifdef ASRP_RECORDING
// 	0x02b, 0x0e25	 , VAL_SHORT,	//LDO Config - input 1.200000, output - 0.880000
// 	0x01d, 0x0600 	 , VAL_SHORT,	//TDM0 BCLK 1.5Mhz, 48Khz 16bits 2CH
// 	0x010, 0xd030001 , VAL_LONG,	//Switch to TDM0 BCLK and TL3 frequency = 208 MHz; Keep Uart1 baudrate with 3Mhz
// #else
// 	0x02b, 0x0e25	 , VAL_SHORT,	//LDO Config - input 1.200000, output - 0.880000
// 	0x01d, 0x0600 	 , VAL_SHORT,	//TDM0 BCLK 1.5Mhz, 48Khz 16bits 2CH
// 	0x010, 0xd02a001 , VAL_LONG,	//Switch to TDM0 BCLK and TL3 frequency = 184 MHz Keep Uart1 baudrate with 1Mhz
// #endif
// 	0x023, 0x0200	 , VAL_SHORT,
// 	0x00f, 0x4185 	 , VAL_SHORT,	//ASRP_SEC_INIT
// 	0x107, 0x0000	 , VAL_SHORT,
// 	0x109, 0x0000	 , VAL_SHORT,
// 	0x034, 0x0003	 , VAL_SHORT,
// 	0x037, 0x09d3	 , VAL_SHORT,	//Config TDM0 Tx Start
// 	0x005, 0x90000400, VAL_LONG,
// 	0x007, 0x1b800217, VAL_LONG,
// 	0x005, 0x90000404, VAL_LONG,
// 	0x007, 0x00000007, VAL_LONG,
// 	0x005, 0x90000406, VAL_LONG,
// 	0x007, 0x100f001f, VAL_LONG,	//Config TDM0 Tx End
// 	0x031, 0x0000	 , VAL_SHORT,	//Config TDM0 Rx Start
// 	0x036, 0x9852	 , VAL_SHORT,
// 	0x005, 0x0300004c, VAL_LONG,	//TDM RX Bypass is enabled ...
// 	0x007, 0xc8055554, VAL_LONG,
// 	0x01f, 0x0040	 , VAL_SHORT,
// 	0x005, 0x90000002, VAL_LONG,
// 	0x007, 0x00000001, VAL_LONG,
// 	0x005, 0x90000000, VAL_LONG,
// 	0x007, 0x1b800015, VAL_LONG,
// 	0x005, 0x90000004, VAL_LONG,
// 	0x007, 0x00000007, VAL_LONG,
// 	0x005, 0x90000006, VAL_LONG,
// 	0x007, 0x100f001f, VAL_LONG,	//Config TDM0 Rx End
// 	0x304, 0x3811	 , VAL_SHORT,
// 	0x305, 0x21f6	 , VAL_SHORT,	//For Cyberon, Wake word and 500ms history time are both required
// 	0x304, 0x5810	 , VAL_SHORT,
// 	0x038, 0x003a	 , VAL_SHORT,
// 	0x10a, 0x0000	 , VAL_SHORT,
// 	0x10b, 0x059f	 , VAL_SHORT,	//ASRP Tx Output increased by 15 dB
// 	0x011, 0x0ee1	 , VAL_SHORT,
// 	0x011, 0x1eee	 , VAL_SHORT,
// 	0x011, 0x22ee	 , VAL_SHORT,
// 	0x011, 0x3eee	 , VAL_SHORT,
// 	0x011, 0x4eee	 , VAL_SHORT,
// 	0x011, 0x5ee3	 , VAL_SHORT,
// 	0x011, 0x6eee	 , VAL_SHORT,
// 	0x011, 0x7eee	 , VAL_SHORT,
// 	0x013, 0xfff3	 , VAL_SHORT,	//buffering with CP3
// 	0x005, 0x0021	 , VAL_SHORT,
// 	0x024 ,0x0261	 , VAL_SHORT,	//RISING_EDGE, DDF0_DM0, 0dB
// 	0x004, 0xf0a0	 , VAL_SHORT,	//Changed to 6DB as Peter request
// #ifdef ASRP_RECORDING
// 	0x00c, 0xc7d0	 , VAL_SHORT,	//Set Uart1 baudrate with 8Mhz
// 	0x128, 0x0011	 , VAL_SHORT,
// 	0x129, 0x0001	 , VAL_SHORT,
// 	0x005, 0x0000	 , VAL_SHORT,
// 	0x006, 0x0000	 , VAL_SHORT,
// 	0x007, 0x0000	 , VAL_SHORT,
// 	0x030, 0x800a	 , VAL_SHORT,
// 	0x030, 0x0001	 , VAL_SHORT,
// #endif
// 	0x031, 0x0304	 , VAL_SHORT,
// };

COMMAND RT_CMD_D10L_evb_bargein_1mic_enter[]=
{
#ifdef ASRP_RECORDING
	0x02b, 0x0e25	 , VAL_SHORT,	//LDO Config - input 1.200000, output - 0.880000
	0x01d, 0x0600 	 , VAL_SHORT,	//TDM0 BCLK 1.5Mhz, 48Khz 16bits 2CH
	0x010, 0xd03f000 , VAL_LONG,	//Switch to TDM0 BCLK and TL3 frequency = 270 MHz; Keep Uart1 baudrate with 3Mhz
#else
	0x02b, 0x0e25	 , VAL_SHORT,	//LDO Config - input 1.200000, output - 0.880000
	0x01d, 0x0600 	 , VAL_SHORT,	//TDM0 BCLK 1.5Mhz, 48Khz 16bits 2CH
	0x010, 0xd03f000 , VAL_LONG,	//Switch to TDM0 BCLK and TL3 frequency = 270 MHz Keep Uart1 baudrate with 1Mhz
#endif
	0x023, 0x0200	 , VAL_SHORT,	//FW_VAD_TYPE_NO_VAD | DDF_SAMPLE_WIDTH_16_BIT | MIC_SAMPLE_RATE_48K
	0x00f, 0x4185 	 , VAL_SHORT,	//ASRP_SEC_INIT
	0x107, 0x0000	 , VAL_SHORT,
	0x109, 0x0000	 , VAL_SHORT,
	0x034, 0x2003	 , VAL_SHORT,	//ALGO1_EN_FW_MODE_1_AND_MODE_2 | POST_DETECTION_MODE_REMAIN_IN_DETECTION
#if USE_HIGHER_GPIO_AT_3V3
#if TDM_CLK_SHARED
	0x01f, 0x0004	 , VAL_SHORT,	//MUSIC_IN_TDM0 | TDM1_3V3
#else
	0x01f, 0x0024	 , VAL_SHORT,	//MUSIC_IN_TDM0 | TDM1_3V3 | TDM1_IOS_NOT_SHARED
#endif
#else
#if TDM_CLK_SHARED
	0x01f, 0x0004	 , VAL_SHORT,	//MUSIC_IN_TDM0 | TDM1_3V3
#else
	0x01f, 0x0024	 , VAL_SHORT,	//MUSIC_IN_TDM0 | TDM1_3V3 | TDM1_IOS_NOT_SHARED
#endif
#endif
	0x031, 0x0000	 , VAL_SHORT,	//Config TDM0
	0x036, 0x9852	 , VAL_SHORT,	//Config TDM0 Rx Start, CP2
	0x005, 0x90000000, VAL_LONG,
	0x007, 0x1b800015, VAL_LONG,
	0x005, 0x90000004, VAL_LONG,
	0x007, 0x00000007, VAL_LONG,
	0x005, 0x90000006, VAL_LONG,
	0x007, 0x100f001f, VAL_LONG,	//Config TDM0 Rx End
#if TDM1_MASTER
	0x031, 0x0001	 , VAL_SHORT,	//Config TDM1
	0x037, 0x09d2	 , VAL_SHORT,	//Config TDM1 Tx Start, CP2
	0x005, 0x90800400, VAL_LONG,
	0x007, 0x1b800052, VAL_LONG,
	0x005, 0x90800404, VAL_LONG,
	0x007, 0x00241024, VAL_LONG,
	0x005, 0x90800406, VAL_LONG,
	0x007, 0x100f001f, VAL_LONG,
	0x005, 0x9080040a, VAL_LONG,
	0x007, 0x0000000f, VAL_LONG,	//Config TDM1 Tx End
#else
	0x031, 0x0001	 , VAL_SHORT,	//Config TDM1
	0x037, 0x09d2	 , VAL_SHORT,	//Config TDM1 Tx Start, CP2
	0x005, 0x90800400, VAL_LONG,
	0x007, 0x1b800217, VAL_LONG,
	0x005, 0x90800404, VAL_LONG,
	0x007, 0x00000007, VAL_LONG,
	0x005, 0x90800406, VAL_LONG,
	0x007, 0x100f001f, VAL_LONG,	//Config TDM1 Tx End
#endif
	0x304, 0x3811	 , VAL_SHORT,
	0x305, 0x21f6	 , VAL_SHORT,	//For Cyberon, Wake word and 500ms history time are both required
	// 0x304, 0x5810	 , VAL_SHORT,		//Enable VT2 for SED	2022.09.06
	0x038, 0x003a	 , VAL_SHORT,
	0x10a, 0x0000	 , VAL_SHORT,
	0x10b, 0x059f	 , VAL_SHORT,	//ASRP Tx Output increased by 15 dB
	0x011, 0x0ee1	 , VAL_SHORT,
	0x011, 0x1eee	 , VAL_SHORT,
	0x011, 0x22ee	 , VAL_SHORT,
	0x011, 0x3eee	 , VAL_SHORT,
	0x011, 0x4eee	 , VAL_SHORT,
	0x011, 0x5ee1	 , VAL_SHORT,
	0x011, 0x6eee	 , VAL_SHORT,
	0x011, 0x7eee	 , VAL_SHORT,
	0x005, 0x0021	 , VAL_SHORT,
	0x024 ,0x0261	 , VAL_SHORT,	//RISING_EDGE, DDF0_DM0, 0dB
	0x004, 0xf0a0	 , VAL_SHORT,	//Changed to 6DB as Peter request
#ifdef ASRP_RECORDING
	0x00c, 0xc7d0	 , VAL_SHORT,	//Set Uart1 baudrate with 8Mhz
	0x128, 0x0011	 , VAL_SHORT,	//ASRP_REC_CH_TX_IN_1 | ASRP_REC_CH_RX_IN_1
	0x129, 0x0001	 , VAL_SHORT,	//ASRP_REC_CH_TX_OUT_1
	0x005, 0x0000	 , VAL_SHORT,
	0x006, 0x0000	 , VAL_SHORT,
	0x007, 0x0000	 , VAL_SHORT,
	0x030, 0x800a	 , VAL_SHORT,
	0x030, 0x0001	 , VAL_SHORT,
#endif
	0x031, 0x0904	 , VAL_SHORT,
};


int RT_CMD_D10L_evb_bargein_1mic_enter_length = sizeof(RT_CMD_D10L_evb_bargein_1mic_enter)/sizeof(RT_CMD_D10L_evb_bargein_1mic_enter[0]);


COMMAND RT_CMD_D10L_evb_bargein_1mic_exit[]=
{
	0x031, 0x0001	 , VAL_SHORT,
	0x031, 0x0000	 , VAL_SHORT,
	0x01b, 0x0020	 , VAL_SHORT,	//Set MCLK 32.768Khz
	0x010 ,0x09013000, VAL_LONG,   	//TL3 frequency = 110.592 MHz
	0x031, 0x0001	 , VAL_SHORT,
	0x037, 0x0000	 , VAL_SHORT,
	0x036, 0x0000	 , VAL_SHORT,
	0x031, 0x0000	 , VAL_SHORT,
	0x037, 0x0000	 , VAL_SHORT,
	0x036, 0x0000	 , VAL_SHORT,
#ifdef ASRP_RECORDING
	0x128, 0x0000	 , VAL_SHORT,
	0x129, 0x0000	 , VAL_SHORT,
	0x005, 0x0000	 , VAL_SHORT,
	0x006, 0x0000	 , VAL_SHORT,
	0x007, 0x0000	 , VAL_SHORT,
	0x030, 0x0000	 , VAL_SHORT,
	0x030, 0x8400	 , VAL_SHORT,
#endif
	0x034, 0x0000	 , VAL_SHORT,
	0x013, 0xffff	 , VAL_SHORT,
	0x00f, 0x48c4	 , VAL_SHORT,
	0x023, 0x0200	 , VAL_SHORT,
	0x024, 0x0200	 , VAL_SHORT,
	0x005, 0x0000	 , VAL_SHORT,
	0x304, 0x0000	 , VAL_SHORT,
	0x01f, 0x0000	 , VAL_SHORT,
};

#if 0
COMMAND RT_CMD_D10L_evb_vc_1mic_enter[]=
{
#ifdef ASRP_RECORDING
	0x02b, 0x0e25	 , VAL_SHORT,	//LDO Config - input 1.200000, output - 0.880000
	0x01d, 0x0200 	 , VAL_SHORT,	//TDM0 BCLK 0.5Mhz, 16Khz 16bits 2CH
	0x010, 0xd030000 , VAL_LONG,	//Switch to TDM0 BCLK and TL3 frequency = 208 MHz; Keep Uart1 baudrate with 3Mhz
#else
	0x02b, 0x0e25	 , VAL_SHORT,	//LDO Config - input 1.200000, output - 0.880000
	0x01d, 0x0200 	 , VAL_SHORT,	//TDM0 BCLK 0.5Mhz, 16Khz 16bits 2CH
	0x010, 0xd02d000 , VAL_LONG,	//Switch to TDM0 BCLK and TL3 frequency = 196 MHz Keep Uart1 baudrate with 1Mhz
#endif
	0x304, 0x0000	 , VAL_SHORT,
	0x023, 0x0200	 , VAL_SHORT,
	0x031, 0x0000	 , VAL_SHORT,	//Config TDM0
	0x036, 0x9012	 , VAL_SHORT,	//Config TDM0 Rx Start
	0x005, 0x90000000, VAL_LONG,
	0x007, 0x1b800015, VAL_LONG,
	0x005, 0x90000004, VAL_LONG,
	0x007, 0x00000007, VAL_LONG,
	0x005, 0x90000006, VAL_LONG,
	0x007, 0x100f001f, VAL_LONG,	//Config TDM0 Rx End
	0x00f, 0x4185 	 , VAL_SHORT,	//ASRP_SEC_INIT
	0x107, 0x0000	 , VAL_SHORT,	//g_asrp_delay = 0
	0x109, 0x0200	 , VAL_SHORT,
	0x10a, 0x0000	 , VAL_SHORT,
	0x10b, 0x059f	 , VAL_SHORT,	//ASRP Tx Output increased by 15 dB
	0x034, 0x0002	 , VAL_SHORT,
	0x031, 0x0000	 , VAL_SHORT,	//Config TDM0
	0x037, 0x0111	 , VAL_SHORT,	//Config TDM0 Tx Start
	0x005, 0x90000400, VAL_LONG,
	0x007, 0x1b800217, VAL_LONG,
	0x005, 0x90000404, VAL_LONG,
	0x007, 0x00000007, VAL_LONG,
	0x005, 0x90000406, VAL_LONG,
	0x007, 0x100f001f, VAL_LONG,	//Config TDM0 Tx End
	0x031, 0x0001	 , VAL_SHORT,	//Config TDM1
	0x037, 0x0112	 , VAL_SHORT,	//Config TDM1 Tx Start
	0x005, 0x90800400, VAL_LONG,
	0x007, 0x1b800217, VAL_LONG,
	0x005, 0x90800404, VAL_LONG,
	0x007, 0x00000007, VAL_LONG,
	0x005, 0x90800406, VAL_LONG,
	0x007, 0x100f001f, VAL_LONG,	//Config TDM1 Tx End
	0x011, 0x0ee1	 , VAL_SHORT,
	0x011, 0x1eee	 , VAL_SHORT,
	0x011, 0x22ee	 , VAL_SHORT,
	0x011, 0x3eee	 , VAL_SHORT,
	0x011, 0x4eee	 , VAL_SHORT,
	0x011, 0x5ee1	 , VAL_SHORT,
	0x011, 0x6eee	 , VAL_SHORT,
	0x011, 0x72ee	 , VAL_SHORT,
	0x011, 0x82ee	 , VAL_SHORT,
	0x005, 0x0021	 , VAL_SHORT,
	0x024 ,0x0261	 , VAL_SHORT,	//RISING_EDGE, DDF0_DM0, 0dB
	0x004, 0xf0a0	 , VAL_SHORT,	//Changed to 6DB as Peter request
#ifdef ASRP_RECORDING
	0x00c, 0xc7d0	 , VAL_SHORT,	//Set Uart1 baudrate with 8Mhz
	0x128, 0x0051	 , VAL_SHORT,
	0x129, 0x0001	 , VAL_SHORT,
	0x005, 0x0000	 , VAL_SHORT,
	0x006, 0x0000	 , VAL_SHORT,
	0x007, 0x0000	 , VAL_SHORT,
	0x030, 0x800a	 , VAL_SHORT,
	0x030, 0x0001	 , VAL_SHORT,
#endif
	0x031, 0x0b08	 , VAL_SHORT,
};
#endif

COMMAND RT_CMD_D10L_evb_vc_1mic_enter1[]=
{
#ifdef ASRP_RECORDING
	0x02b, 0x0e25	 , VAL_SHORT,	//LDO Config - input 1.200000, output - 0.880000
	0x01d, 0x0200 	 , VAL_SHORT,	//TDM0 BCLK 0.5Mhz, 16Khz 16bits 2CH
	0x010, 0xd030000 , VAL_LONG,	//Switch to TDM0 BCLK and TL3 frequency = 208 MHz; Keep Uart1 baudrate with 3Mhz
#else
	0x02b, 0x0e25	 , VAL_SHORT,	//LDO Config - input 1.200000, output - 0.880000
	0x01d, 0x0200 	 , VAL_SHORT,	//TDM0 BCLK 0.5Mhz, 16Khz 16bits 2CH
	0x010, 0xd02d000 , VAL_LONG,	//Switch to TDM0 BCLK and TL3 frequency = 196 MHz Keep Uart1 baudrate with 1Mhz
#endif
	0x304, 0x0000	 , VAL_SHORT,
	0x023, 0x0200	 , VAL_SHORT,
};

COMMAND RT_CMD_D10L_evb_vc_1mic_enter2[]=
{
	0x031, 0x0000	 , VAL_SHORT,	//Config TDM0
	0x036, 0x9012	 , VAL_SHORT,	//Config TDM0 Rx Start
	0x005, 0x90000000, VAL_LONG,
	0x007, 0x1b800015, VAL_LONG,
	0x005, 0x90000004, VAL_LONG,
	0x007, 0x00000007, VAL_LONG,
	0x005, 0x90000006, VAL_LONG,
	0x007, 0x100f001f, VAL_LONG,	//Config TDM0 Rx End
	0x00f, 0x4185 	 , VAL_SHORT,	//ASRP_SEC_INIT
	0x107, 0x0000	 , VAL_SHORT,	//g_asrp_delay = 0
	0x109, 0x0200	 , VAL_SHORT,
	0x10a, 0x0000	 , VAL_SHORT,
	0x10b, 0x059f	 , VAL_SHORT,	//ASRP Tx Output increased by 15 dB
	0x034, 0x0002	 , VAL_SHORT,
	0x031, 0x0000	 , VAL_SHORT,	//Config TDM0
	0x037, 0x0111	 , VAL_SHORT,	//Config TDM0 Tx Start
	0x005, 0x90000400, VAL_LONG,
	0x007, 0x1b800217, VAL_LONG,
	0x005, 0x90000404, VAL_LONG,
	0x007, 0x00000007, VAL_LONG,
	0x005, 0x90000406, VAL_LONG,
	0x007, 0x100f001f, VAL_LONG,	//Config TDM0 Tx End
	0x031, 0x0001	 , VAL_SHORT,	//Config TDM1
	0x037, 0x0112	 , VAL_SHORT,	//Config TDM1 Tx Start
	0x005, 0x90800400, VAL_LONG,
	0x007, 0x1b800217, VAL_LONG,
	0x005, 0x90800404, VAL_LONG,
	0x007, 0x00000007, VAL_LONG,
	0x005, 0x90800406, VAL_LONG,
	0x007, 0x100f001f, VAL_LONG,	//Config TDM1 Tx End
	0x011, 0x0ee1	 , VAL_SHORT,
	0x011, 0x1eee	 , VAL_SHORT,
	0x011, 0x22ee	 , VAL_SHORT,
	0x011, 0x3eee	 , VAL_SHORT,
	0x011, 0x4eee	 , VAL_SHORT,
	0x011, 0x5ee1	 , VAL_SHORT,
	0x011, 0x6eee	 , VAL_SHORT,
	0x011, 0x72ee	 , VAL_SHORT,
	0x011, 0x82ee	 , VAL_SHORT,
	0x005, 0x0021	 , VAL_SHORT,
	0x024 ,0x0261	 , VAL_SHORT,	//RISING_EDGE, DDF0_DM0, 0dB
	0x004, 0xf0a0	 , VAL_SHORT,	//Changed to 6DB as Peter request
#ifdef ASRP_RECORDING
	0x00c, 0xc7d0	 , VAL_SHORT,	//Set Uart1 baudrate with 8Mhz
	0x128, 0x0051	 , VAL_SHORT,
	0x129, 0x0001	 , VAL_SHORT,
	0x005, 0x0000	 , VAL_SHORT,
	0x006, 0x0000	 , VAL_SHORT,
	0x007, 0x0000	 , VAL_SHORT,
	0x030, 0x800a	 , VAL_SHORT,
	0x030, 0x0001	 , VAL_SHORT,
#endif
	0x031, 0x0b08	 , VAL_SHORT,
};

COMMAND RT_CMD_D10L_evb_vc_1mic_exit[]=
{
	0x031, 0x0001	 , VAL_SHORT,
	0x031, 0x0000	 , VAL_SHORT,
	0x01b, 0x0020	 , VAL_SHORT,	//Set MCLK 32.768Khz
	0x010 ,0x09013000, VAL_LONG,   	//TL3 frequency = 110.592 MHz
	0x031, 0x0001	 , VAL_SHORT,
	0x037, 0x0000	 , VAL_SHORT,
	0x036, 0x0000	 , VAL_SHORT,
	0x031, 0x0000	 , VAL_SHORT,
	0x037, 0x0000	 , VAL_SHORT,
	0x036, 0x0000	 , VAL_SHORT,
#ifdef ASRP_RECORDING
	0x128, 0x0000	 , VAL_SHORT,
	0x129, 0x0000	 , VAL_SHORT,
	0x005, 0x0000	 , VAL_SHORT,
	0x006, 0x0000	 , VAL_SHORT,
	0x007, 0x0000	 , VAL_SHORT,
	0x030, 0x0000	 , VAL_SHORT,
	0x030, 0x8400	 , VAL_SHORT,
#endif
	0x034, 0x0000	 , VAL_SHORT,
	0x013, 0xffff	 , VAL_SHORT,
	0x00f, 0x48c4	 , VAL_SHORT,
	0x023, 0x0200	 , VAL_SHORT,
	0x024, 0x0200	 , VAL_SHORT,
	0x005, 0x0000	 , VAL_SHORT,
	0x304, 0x0000	 , VAL_SHORT,
	0x01f, 0x0000	 , VAL_SHORT,
};

COMMAND RT_CMD_D10L_evb_production_test_enter[]=
{
	//0x02b, 0x0000  , VAL_SHORT,	//Set LDO_EN disabled
	0x00c, 0x83e8	 , VAL_SHORT,	//Set Uart1 baudrate 1Mhz
	0x010, 0x100d000 , VAL_LONG,	//TL3 frequency = 86.016 MHz
	0x304, 0x1810	 , VAL_SHORT,
	0x305, 0x0000	 , VAL_SHORT,
	0x038, 0x0000	 , VAL_SHORT,
	0x013, 0xfff0	 , VAL_SHORT,	//buffering with 1 Mic
	0x034, 0x0000	 , VAL_SHORT,
	0x024 ,0x0261	, VAL_SHORT,	//RISING_EDGE, DDF0_DM0, 0dB
	0x004, 0xf0D0	 , VAL_SHORT,	//Changed to 6DB as Peter request
	0x001, 0x0001	 , VAL_SHORT,
};

COMMAND RT_CMD_D10L_evb_production_test_exit[]=
{
	0x001, 0x0000	 , VAL_SHORT,
	0x023, 0x0000	 , VAL_SHORT,
	0x024, 0x0200	 , VAL_SHORT,
	0x010 ,0x1013000 , VAL_LONG,   	//TL3 frequency = 110.592 MHz
};


char cyberon_trigger_commands[3][60] = 
{
	"Hey RAK Star",
	"Hey Helium",
	"Hey RAK Cloud"
};
char g_cyberon_group1_commands[28][60] = 
{
	"Play Music",			//0
	"Play Music",			//1	
	"Play Music",			//2
	"Play Music",			//3

	"Stop Music",			//4
	"Stop Music",			//5
	"Stop Music",			//6
	"Stop Music",			//7

	"Previous Song",		//8

	"Next song",			//9
	"Next song",			//10

	"Volume Up",			//11	
	"Volume Down",			//12
	"Stop",					//13
	"Pause",				//14

	"Next",					//15
	"Next",					//16

	"Resume",				//17
	"Resume",				//18
	"Resume",				//19
	"Resume",				//20
	"Resume",				//21

	"Previous",				//22

	"Volume Up",			//23
	"Volume Down",			//24

	"Increase Volume",		//25

	"Decrease Volume",		//26
	"Decrease Volume"		//27
};
char g_cyberon_group2_commands[17][60] = 
{
	"Turn Lights on",		//0
	"Turn Lights on",		//1

	"Turn Lights off",		//2

	"Lights on" ,			//3
	"Lights on" ,			//4

	"Lights off" ,			//5
	"Turn lights up",		//6
	"Turn lights down",		//7
	"Dim lights",			//8
	"Brighten lights",		//9
	"Lights Red",			//10
	"Lights Blue",			//11
	"Lights Green",			//12
	"Lights Yellow",		//13

	"Lights Orange",		//14
	"Lights Orange",		//15

	"Lights Pink"			//16
};
char g_cyberon_group3_commands[29][60] = 
{
	"What is today's HNT?",								//0
	"What is today's HNT?",								//1
	"What is today's HNT?",								//2
	"What is today's HNT?",								//3

	"How many hotspots are deployed today?",			//4
	"How many hotspots are deployed today?",			//5
	"How many hotspots are deployed today?",			//6
	"How many hotspots are deployed today?",			//7

	"Which countries have Helium hotspots deployed?",	//8
	"Which countries have Helium hotspots deployed?",	//9

	"What is the price of HNT today?",					//10
	"What is the price of HNT today?",					//11
	"What is the price of HNT today?",					//12
	"What is the price of HNT today?",					//13
	"What is the price of HNT today?",					//14
	"What is the price of HNT today?",					//15
	"What is the price of HNT today?",					//16
	"What is the price of HNT today?",					//17

	"What are the latest people's network updates?",	//18
	"What are the latest people's network updates?",	//19
	"What are the latest people's network updates?",	//20
	"What are the latest people's network updates?",	//21
	"What are the latest people's network updates?",	//22
	"What are the latest people's network updates?",	//23
	"What are the latest people's network updates?",	//24
	"What are the latest people's network updates?",	//25

	"Is there Helium coverage",							//26
	"Is there Helium coverage",							//27
	"Is there Helium coverage"							//28
};
char g_cyberon_group4_commands[13][60] = 
{
	"Connect",					//0
	"Disconnect",				//1

	"Report Location",			//2
	"Report Location",			//3

	"Get Status",				//4
	"Get Status",				//5
	"Get Status",				//6
	"Get Status",				//7

	"Report Battery",			//8
	"Report Battery",			//9

	"Report Environment",		//10
	"Report Environment",		//11

	"Reset"						//12
};
