#ifndef _DSPGREG_H_
#define _DSPGREG_H_


/* Command Registers on DBMDX */


 
/* Registers on DBMDx chip*/
#define FW_VERSION_NUMBER  0x00
#define OPERATION_MODE  0x01
/*------------------------*/
#define IDLE_MODE 0x0
#define DETECTION_MODE 0x1
#define BUFFERING_MODE 0x2
#define HIBERNATE_MODE 0x6

#define MIC_MUTE_DUR 0x3
/*------------------------*/
#define KEEP_CURRENT_DUR 0x800
#define DISCARD_SAMPLES 0x800
#define MUTE_MIC1 0x1000
#define MUTE_MIC2 0x2000
#define MUTE_MIC3 0x3000
#define MUTE_MIC4 0x4000
#define MUTE_ALL_MICROPHONES 0x0
#define MIC_MUTE_TIME_10MS 0xA
#define MIC_MUTE_TIME_6MS 0x6
#define MIC_MUTE_TIME_8MS 0x8
#define MIC_MUTE_TIME_1MS 0x1
#define NO_MUTE 0xA


#define DIGITAL_GAIN  0x04
#define 		KEEP_CURRENT_GAIN  0x0800
#define 		GAIN_AFFECTS_MIC1  0x1000
#define 		GAIN_AFFECTS_MIC2  0x2000
#define 		GAIN_AFFECTS_MIC3  0x3000
#define 		GAIN_AFFECTS_MIC4  0x4000
#define 		GAIN_AFFECTS_ALL_MICROPHONES  0xF000
#define 		DIGITAL_GAIN_3_DB  0x30


#define PRIMARY_ACOUSTIC_MODEL_SIZE 	0x02
#define SECONDARY_ACOUSTIC_MODEL_SIZE 	0x03


#define IO_PORT_ADDRESS_LOW  0x05
#define 		CONFIG_VESPER_MIC_ENERGY 0x5
#define 		CONFIG_VESPER_MIC_ENERGY_LOW 0x5
#define 		FW_MINOR_NUMBER 0x5

#define IO_PORT_ADDRESS_HIGH  0x06
#define 		CONFIG_VESPER_MIC_ENERGY_HIGH 0x6
#define 		FW_RC_NUMBER 0x6

#define IO_PORT_VALUE_LOW  0x07
#define IO_PORT_VALUE_HIGH  0x08

#define CONFIG_VESPER_MIC_ENERGY 0x5
#define CONFIG_VESPER_MIC_ENERGY_LOW 0x5
#define CONFIG_VESPER_MIC_ENERGY_HIGH 0x6


#define AUDIO_BUFFER_SIZE  0x09
#define 		AUDIO_BUFFER_1_SEC  0x0800
#define 		AUDIO_BUFFER_1_5_SEC  0x0C00
#define 		AUDIO_BUFFER_2_SEC  0x1000
#define 		AUDIO_BUFFER_2_5_SEC  0x1400
#define 		AUDIO_BUFFER_3_SEC  0x1800
#define 		AUDIO_BUFFER_5_SEC  0x36AE		/*reference setting for D4A*/	

#define NUMBER_OF_SAMPLES_IN_BUFFER  0x0A
#define LAST_MAXIMUM_SAMPLE_VALUE  0x0B
#define UART_BAUD_RATE  0x0C
#define 		CHANGE_UART1_BAUDRATE  0x8000
#define 		CHANGE_UART0_BAUDRATE  0x0
#define 		UART0_UART_BAUD_RATE_115_200_hz  0x480
#define 		UART0_UART_BAUD_RATE_230_400_hz  0x900
#define 		UART0_UART_BAUD_RATE_460_800_hz  0x1200
#define 		UART0_UART_BAUD_RATE_780_000_hz  0x1E78
#define 		UART0_UART_BAUD_RATE_1_Mhz  0x2710
#define 		UART0_UART_BAUD_RATE_2_Mhz  0x4e20                                            
#define 		UART0_UART_BAUD_RATE_3_Mhz  0x7530                                            
#define 		UART1_UART_BAUD_RATE_100_Khz  0x64                                            
#define 		UART1_UART_BAUD_RATE_200_Khz  0xc8                                            
#define 		UART1_UART_BAUD_RATE_400_Khz  0x190                                           
#define 		UART1_UART_BAUD_RATE_500_Khz  0x1F4                                           
#define 		UART1_UART_BAUD_RATE_800_Khz  0x320                                           
#define 		UART1_UART_BAUD_RATE_1_Mhz  0x3e8                                             
#define 		UART1_UART_BAUD_RATE_2_Mhz  0x7d0                                             
#define 		UART1_UART_BAUD_RATE_3_Mhz  0xbb8                                             
#define 		UART1_UART_BAUD_RATE_4_Mhz  0xfa0  


#define FW_STATUS_INDEX  0x0D
#define 		READ_FW_ERROR_COUNTER  0x0000
#define 		GET_AMOUNT_OF_FREE_DATA_MEMORY  0x0001
#define 		GET_AMOUNT_OF_FREE_AHB_MEMORY  0x0002
#define 		GET_AMOUNT_OF_ALLOC_BLOCKS_IN_DTCM  0x0003
#define 		GET_AMOUNT_OF_ALLOC_BLOCKS_IN_AHB  0x0004
#define 		GET_MIC_ENR_LEVEL  0x0005


#define MICROPHONE_AUTO_SET  0x0E
#define 		AUTO_EN_DIS_DDF_BLOCK_D2_AFE_DDF_D4_SD_DDF  0x0001
#define 		AUTO_EN_DIS_DDF_BLOCK_D2_PDM0L_D4_DDF0_D6_DDF0  0x0002
#define 		AUTO_EN_DIS_DDF_BLOCK_D2_PDM0R_D4_DDF1_D6_DDF1  0x0004
#define 		AUTO_EN_DIS_DDF_BLOCK_D2_PDM1L_D4_SAR_DDF_D6_SAR_DDF  0x0008
#define 		AUTO_EN_DIS_DDF_BLOCK_D2_PDM1R  0x0010
#define 		AUTO_EN_DIS_DDF_BLOCK_D4_SD_ADC  0x0020
#define 		AUTO_EN_DIS_DDF_BLOCK_D4_SAR_ADC_D6_SAR_ADC  0x0040
#define 		AUTO_EN_DIS_DDF_BLOCK_D4_DM1_D6_DM1  0x0080
#define 		AUTO_EN_DIS_DDF_BLOCK_D4_DM0_D6_DM0  0x0100
#define 		AUTOSET_EVENT_DISABLE_AUTO_SET  0x0000
#define 		AUTOSET_EVENT_ENABLE_ON_VAD  0x0200
#define 		AUTOSET_EVENT_ENABLE_ON_VT_DETECTION  0x0400

#define LOAD_BINARY_FILE  0x0F
#define 		FILE_TYPE_DTE_PRIM_MODEL		  0x0
#define 		FILE_TYPE_DTE_SEC_MODEL			  0x1
#define 		FILE_TYPE_ASRP_PARAMS_PRIM_INIT 	  0x4
#define 		FILE_TYPE_ASRP_SEC_INIT			  0x5
#define 		FILE_TYPE_NNET_BINS				  0x6
#define 		FILE_TYPE_NNET_FEATURES			  0x7
#define 		FILE_TYPE_PRIM_AM_FIRST_BLOCK	  0X10
#define 		FILE_TYPE_DTE_LOAD_TO_PREALLOC	  0x12
#define 		FILE_TYPE_FW_CMD_SEQ			  0x30
#define 		FILE_TYPE_GENERIC_BIN			  0x31

#define 		FILE_NO_OP 						  0x0
#define 		FILE_LOAD_OR_REL_ONLY 			  0x40
#define 		FILE_INIT_OR_KILL_ONLY 	  		  0x80
#define 		FILE_LOAD_INIT_OR_KILL_REL 		  0xC0

#define 		MEM_TYPE_DTCM   0x0
#define 		MEM_TYPE_AHB   0x100
#define 		BLK_START_NUM_0   0x0
#define 		BLK_START_NUM_1   0x200
#define 		OP_TYPE_LOAD_FILE   0x0
#define 		OP_TYPE_RELEASE_FILE   0x800
#define 		LOAD_ENGINE_TYPE_VT1   0x1000
#define 		LOAD_ENGINE_TYPE_VT2   0x2000
#define 		LOAD_ENGINE_TYPE_VT3   0x3000
#define 		LOAD_ENGINE_TYPE_ASRP   0x4000
#define 		LOAD_ENGINE_TYPE_SED   0x5000


#define DSP_CLOCK_CONFIG  0x10


#define           AHB_DIV_1  0x0000
#define           AHB_DIV_2  0x0001
#define           AHB_DIV_3  0x0002
#define           AHB_DIV_4  0x0003
#define           AHB_DIV_5  0x0004
#define           AHB_DIV_6  0x0005
#define           AHB_DIV_7  0x0006
#define           AHB_DIV_8  0x0007
#define           AHB_DIV_9  0x0008
#define           AHB_DIV_10  0x0009
#define           AHB_DIV_11  0x000A
#define           AHB_DIV_12  0x000B
#define           AHB_DIV_13  0x000C
#define           AHB_DIV_14  0x000D
#define           AHB_DIV_15  0x000E
#define           AHB_DIV_16  0x000F

#define           APB_DIV_1  0x0000
#define           APB_DIV_2  0x0010
#define           APB_DIV_3  0x0020
#define           APB_DIV_4  0x0030
#define           APB_DIV_5  0x0040
#define           APB_DIV_6  0x0050
#define           APB_DIV_7  0x0060
#define           APB_DIV_8  0x0070
#define           APB_DIV_9  0x0080
#define           APB_DIV_10  0x0090
#define           APB_DIV_11  0x00A0
#define           APB_DIV_12  0x00B0
#define           APB_DIV_13  0x00C0
#define           APB_DIV_14  0x00D0
#define           APB_DIV_15  0x00E0
#define           APB_DIV_16  0x00F0

#define           TL3_DIV_1  0x0000
#define           TL3_DIV_2  0x0100
#define           TL3_DIV_3  0x0200
#define           TL3_DIV_4  0x0300
#define           TL3_DIV_5  0x0400
#define           TL3_DIV_6  0x0500
#define           TL3_DIV_7  0x0600
#define           TL3_DIV_8  0x0700
#define           TL3_DIV_9  0x0800
#define           TL3_DIV_10  0x0900
#define           TL3_DIV_11  0x0A00
#define           TL3_DIV_12  0x0B00
#define           TL3_DIV_13  0x0C00
#define           TL3_DIV_14  0x0D00
#define           TL3_DIV_15  0x0E00
#define           TL3_DIV_16  0x0F00

#define           PLL_STEP_1  0x1000
#define           PLL_STEP_2  0x2000
#define           PLL_STEP_3  0x3000
#define           PLL_STEP_4  0x4000
#define           PLL_STEP_5  0x5000
#define           PLL_STEP_6  0x6000
#define           PLL_STEP_7  0x7000
#define           PLL_STEP_8  0x8000
#define           PLL_STEP_9  0x9000
#define           PLL_STEP_10  0xA000
#define           PLL_STEP_11  0xB000
#define           PLL_STEP_12  0xC000
#define           PLL_STEP_13  0xD000
#define           PLL_STEP_14  0xE000

#define           USE_PLL_STEP_FROM_REG_0x1E  0xF000

#define AUDIO_PROCESSING_ROUTING  0x11


#define           IO_SET_0 0x0000
#define           IO_SET_1 0x1000
#define           IO_SET_2 0x2000
#define           IO_SET_3 0x3000
#define           IO_SET_4 0x4000
#define           IO_SET_5 0x5000
#define           IO_SET_6 0x6000
#define           IO_SET_7 0x7000
#define           IO_SET_8 0x8000
#define           IO_3N_0_CP_0 0x0000
#define           IO_3N_0_CP_1 0x0001
#define           IO_3N_0_CP_2 0x0002
#define           IO_3N_0_CP_3 0x0003
#define           IO_3N_0_CP_4 0x0004
#define           IO_3N_0_CP_5 0x0005
#define           IO_3N_0_CP_6 0x0006
#define           IO_3N_0_CP_7 0x0007
#define           IO_3N_0_NO_CP 0x000E
#define           IO_3N_1_CP_0 0x0000
#define           IO_3N_1_CP_1 0x0010
#define           IO_3N_1_CP_2 0x0020
#define           IO_3N_1_CP_3 0x0030
#define           IO_3N_1_CP_4 0x0040
#define           IO_3N_1_CP_5 0x0050
#define           IO_3N_1_CP_6 0x0060
#define           IO_3N_1_CP_7 0x0070
#define           IO_3N_1_NO_CP 0x00E0
#define           IO_3N_2_CP_0 0x0000
#define           IO_3N_2_CP_1 0x0100
#define           IO_3N_2_CP_2 0x0200
#define           IO_3N_2_CP_3 0x0300
#define           IO_3N_2_CP_4 0x0400
#define           IO_3N_2_CP_5 0x0500
#define           IO_3N_2_CP_6 0x0600
#define           IO_3N_2_CP_7 0x0700
#define           IO_3N_2_NO_CP 0x0E00


#define EVENTS_INDICATIONS_EN_CFG  0x12


#define AUDIO_STREAMING_SOURCE_SELECT  0x13
/*#define NO_STREAMING  0xFFFF*/

#define            NO_STREAM_CH_1  0x000F
#define            STREAM_CH_1_CP_0  0x0000
#define            STREAM_CH_1_CP_1  0x0001
#define            STREAM_CH_1_CP_2  0x0002
#define            STREAM_CH_1_CP_3  0x0003
#define            STREAM_CH_1_CP_4  0x0004
#define            STREAM_CH_1_CP_5  0x0005
#define            STREAM_CH_1_CP_6  0x0006
#define            STREAM_CH_1_CP_7  0x0007
#define            NO_STREAM_CH_2  0x00F0
#define            STREAM_CH_2_CP_0  0x0000
#define            STREAM_CH_2_CP_1  0x0010
#define            STREAM_CH_2_CP_2  0x0020
#define            STREAM_CH_2_CP_3  0x0030
#define            STREAM_CH_2_CP_4  0x0040
#define            STREAM_CH_2_CP_5  0x0050
#define            STREAM_CH_2_CP_6  0x0060
#define            STREAM_CH_2_CP_7  0x0070
#define            NO_STREAM_CH_3  0x0F00
#define            STREAM_CH_3_CP_0  0x0000
#define            STREAM_CH_3_CP_1  0x0100
#define            STREAM_CH_3_CP_2  0x0200
#define            STREAM_CH_3_CP_3  0x0300
#define            STREAM_CH_3_CP_4  0x0400
#define            STREAM_CH_3_CP_5  0x0500
#define            STREAM_CH_3_CP_6  0x0600
#define            STREAM_CH_3_CP_7  0x0700
#define            NO_STREAM_CH_4  0xF000
#define            STREAM_CH_4_CP_0  0x0000
#define            STREAM_CH_4_CP_1  0x1000
#define            STREAM_CH_4_CP_2  0x2000
#define            STREAM_CH_4_CP_3  0x3000
#define            STREAM_CH_4_CP_4  0x4000
#define            STREAM_CH_4_CP_5  0x5000
#define            STREAM_CH_4_CP_6  0x6000
#define            STREAM_CH_4_CP_7  0x7000


#define DETECT_AND_SYS_EVENTS  0x14
#define            PWRUP_COMP  0x1
#define            VT1_DET  0x2
#define            VT2_DET  0x4
#define            VT3_DET  0x8
#define            AEP_DET  0x10
#define            WARNING_EVENT  0x4000
#define            ERROR_EVENT  0x8000


#define EVENTS_INDICATION_GPIO  0x15
#define            EVENTS_GPIO_NUM_1  0x1
#define            EVENTS_GPIO_NUM_4  0x4
#define            EVENTS_GPIO_NUM_14  0xe
#define            EVENTS_GPIO_NUM_16  0x10
#define            EVENTS_GPIO_NUM_15  0xf
#define            EVENTS_GPIO_NUM_20  0x14
#define            EVENTS_GPIO_POL_HIGH  0x0
#define            EVENTS_GPIO_POL_LOW  0x40
#define            EN_EVENTS_GPIO  0x80
#define            DIS_EVENTS_GPIO  0x0
#define            FW_READY_GPIO_NUM_1  0x100
#define            FW_READY_GPIO_NUM_4  0x400
#define            FW_READY_GPIO_NUM_14  0xe00
#define            FW_READY_GPIO_NUM_16  0x1000
#define            FW_READY_GPIO_NUM_15  0xf00
#define            FW_READY_GPIO_NUM_10  0xa00
#define            FW_READY_GPIO_NUM_20  0x1400
#define            FW_READY_GPIO_POL_HIGH  0x0
#define            FW_READY_GPIO_POL_LOW  0x4000
#define            EN_FW_READY_GPIO  0x8000
#define            DIS_FW_READY_GPIO  0x0
                               

#define MICROPHONE_ANALOG_GAIN  0x16
#define            SD_GAIN_EN   0x8000
#define            SD_GAIN_MINUS_4_DB   0x0
#define            SD_GAIN_MINUS_2_DB   0x100
#define            SD_GAIN_0_DB   0x200
#define            SD_GAIN_2_DB   0x300
#define            SD_GAIN_4_DB   0x400
#define            SD_GAIN_6_DB   0x500
#define            SD_GAIN_8_DB   0x600
#define            SD_GAIN_10_DB   0x700
#define            SD_GAIN_12_DB   0x800
#define            SD_GAIN_14_DB   0x900
#define            SD_GAIN_16_DB   0xa00
#define            SD_GAIN_18_DB   0xb00
#define            SD_GAIN_20_DB   0xc00
#define            SD_GAIN_22_DB   0xd00
#define            SD_GAIN_24_DB   0xe00
#define            SD_GAIN_26_DB   0xf00
#define            SD_GAIN_28_DB   0x1000
#define            SD_GAIN_30_DB   0x1100
#define            SD_GAIN_32_DB   0x1200
#define            SD_GAIN_34_DB   0x1300
#define            SD_GAIN_36_DB   0x1400
#define            SAR0_GAIN_EN   0x100
#define            SAR1_GAIN_EN   0x200
#define            SAR1_GAIN_0_DB   0x0
#define            SAR1_GAIN_6_DB   0x10
#define            SAR1_GAIN_12_DB   0x20
#define            SAR1_GAIN_14_DB   0x30
#define            SAR1_GAIN_15_6_DB   0x40
#define            SAR1_GAIN_18_3_DB   0x50
#define            SAR1_GAIN_19_9_DB   0x60
#define            SAR1_GAIN_20_8_DB   0x70
#define            SAR1_GAIN_21_8_DB   0x80
#define            SAR1_GAIN_22_9_DB   0x90
#define            SAR1_GAIN_24_4_DB   0xa0
#define            SAR1_GAIN_25_6_DB   0xb0
#define            SAR1_GAIN_27_2_DB   0xc0
#define            SAR1_GAIN_29_2_DB   0xd0
#define            SAR1_GAIN_31_8_DB   0xe0
#define            SAR1_GAIN_35_4_DB   0xf0
#define            SAR0_GAIN_0_DB   0x0
#define            SAR0_GAIN_6_DB   0x1
#define            SAR0_GAIN_12_DB   0x2
#define            SAR0_GAIN_14_DB   0x3
#define            SAR0_GAIN_15_6_DB   0x4
#define            SAR0_GAIN_18_3_DB   0x5
#define            SAR0_GAIN_19_9_DB   0x6
#define            SAR0_GAIN_20_8_DB   0x7
#define            SAR0_GAIN_21_8_DB   0x8
#define            SAR0_GAIN_22_9_DB   0x9
#define            SAR0_GAIN_24_4_DB   0xa
#define            SAR0_GAIN_25_6_DB   0xb
#define            SAR0_GAIN_27_2_DB   0xc
#define            SAR0_GAIN_29_2_DB   0xd
#define            SAR0_GAIN_31_8_DB   0xe
#define            SAR0_GAIN_35_4_DB   0xf


#define FW_DEBUG_REGISTER_2 0x17
#define PRINT_TDM_SW_SYNC_PATTERN 0x1000
#define FORCE_DETECTION_INTERRUPT 0x80

#define FW_DEBUG_REGISTER  0x18
#define            DUMP_APB_REGS  0x0001
#define            DUMP_AHB_REGS  0x0002
#define            FW_PRINT_CLOCK_SETTING  0x0004
#define            TOGGLE_UART_DEBUG_PRINTS  0x0005
#define            PRINT_MEM_INFO  0x000a
#define            PRINT_MIPS_METER  0x0014
 
#define CHIP_ID_NUMBER  0x19
#define HPF_ENABLE_DISABLE  0x1A

#define            IIR_HPF_EN  0x0001
#define            DC_REMOVE_COARSE_EN  0x0002
#define            DC_REMOVE_FINE_EN  0x0004
#define            IIR_HPF_CUTOFF_FREQ_60HZ  0x0000
#define            IIR_HPF_CUTOFF_FREQ_30HZ  0x01000

#define MASTER_CLOCK_FREQUENCY  0x1B
#define            MCLK_32768Hz  0x0020
#define            MCLK_12288000Hz  0x3000
#define            MCLK_19200000Hz  0x4B00
#define            MCLK_24576000Hz  0x6000

#define I2C_SERVICE  0x1C
#define I2C_SERVICE_CONFIGURE_ADDRESS 0x1000
#define I2C_SERVICE_READ_REGISTER 0x2000
#define I2C_SERVICE_WRITE_REGISTER 0x3000
#define I2C_SERVICE_UPDATE_VALUE 0x5000
#define I2C_SERVICE_USE_D10_SDA_GPIO_11 0x0000
#define I2C_SERVICE_USE_D10_SDA_GPIO_14 0x0400
#define I2C_VESPER_MIC_SLAVE_ADDRESS_LEFT_CHANNEL 0x60
#define I2C_VESPER_MIC_SLAVE_ADDRESS_RIGHT_CHANNEL 0x61
#define I2C_VESPER_REGISTER0 0x0
#define I2C_VESPER_REGISTER1 0x1
#define I2C_VESPER_REGISTER2 0x2
#define I2C_VESPER_REGISTER3 0x3
#define I2C_VESPER_REGISTER4 0x4
#define I2C_VESPER_REGISTER5 0x5


#define VESPER_3011_WOS_THRESHOLD_6_0_DBSPL 0x1
#define VESPER_3011_WOS_THRESHOLD_9_5_DBSPL 0x2
#define VESPER_3011_WOS_THRESHOLD_12_DBSPL 0x3
#define VESPER_3011_WOS_THRESHOLD_14_DBSPL 0x4
#define VESPER_3011_WOS_THRESHOLD_15_5_DBSPL 0x5
#define VESPER_3011_WOS_THRESHOLD_16_9_DBSPL 0x6
#define VESPER_3011_WOS_THRESHOLD_18_DBSPL 0x7


#define TDM_SCLK_CLOCK_FREQUENCY  0x1D
#define DSP_CLOCK_CONFIG_EXTENSION  0x1E


#define            CLK_EXT_PLL_STEP_1  0x0001
#define            CLK_EXT_PLL_STEP_2  0x0002
#define            CLK_EXT_PLL_STEP_3  0x0003
#define            CLK_EXT_PLL_STEP_4  0x0004
#define            CLK_EXT_PLL_STEP_5  0x0005
#define            CLK_EXT_PLL_STEP_6  0x0006
#define            CLK_EXT_PLL_STEP_7  0x0007
#define            CLK_EXT_PLL_STEP_8  0x0008
#define            CLK_EXT_PLL_STEP_9  0x0009
#define            CLK_EXT_PLL_STEP_10  0x000A
#define            CLK_EXT_PLL_STEP_11  0x000B
#define            CLK_EXT_PLL_STEP_12  0x000C
#define            CLK_EXT_PLL_STEP_13  0x000D
#define            CLK_EXT_PLL_STEP_14  0x000E
#define            CLK_EXT_PLL_STEP_15  0x000F
#define            CLK_EXT_PLL_STEP_16  0x0010
#define            CLK_EXT_PLL_STEP_17  0x0011
#define            CLK_EXT_PLL_STEP_18  0x0012
#define            CLK_EXT_PLL_STEP_19  0x0013
#define            CLK_EXT_PLL_STEP_20  0x0014
#define            CLK_EXT_PLL_STEP_21  0x0015
#define            CLK_EXT_PLL_STEP_22  0x0016
#define            CLK_EXT_PLL_STEP_23  0x0017
#define            CLK_EXT_PLL_STEP_24  0x0018
#define            CLK_EXT_PLL_STEP_25  0x0019
#define            CLK_EXT_PLL_STEP_26  0x001A
#define            CLK_EXT_PLL_STEP_27  0x001B
#define            CLK_EXT_PLL_STEP_28  0x001C
#define            CLK_EXT_PLL_STEP_29  0x001D
#define            CLK_EXT_PLL_STEP_30  0x001E
#define            CLK_EXT_PLL_STEP_31  0x001F



#define OSC_FREE_RUN  0x0800
#define            PLL_OSC_SEL_PLL_OSC_OFF  0x0000
#define            PLL_OSC_SEL_USE_PLL  0x1000
#define            PLL_OSC_SEL_USE_OSC  0x2000
#define            CLK_SEL_MCLK_IN  0x0000
#define            CLK_SEL_TDM0_SCLK  0x4000
#define            SRC_SET_KEEP_CUR_CLK_SRC  0x0000
#define            SRC_SET_CONFIG_CLK_ACCORDING_TO_CLK_SEL  0x8000
#define            FORM_SEL_0 0x0
#define            FORM_SEL_1 0x10000
#define            FORM_SEL_2 0x20000

                               
#define AUDIO_ROUTING_CONFIG  0x1F
#define            D2_SET_TDM0_FOR_BYPASS  0x0004
#define            D2_SET_TDM1_FOR_BYPASS  0x0008
#define            D2_SET_TDM2_FOR_BYPASS  0x0010
#define            D2_SET_TDM3_FOR_BYPASS  0x0020
#define            TDM_BYPASS_EN  0x0040
#define            TDM_BYPASS_MODE_TDM0_TO_TDM1_TDM2_TO_TDM3  0x0000
#define            TDM_BYPASS_MODE_TDM0_TO_TDM3_TDM2_TO_TDM1  0x0080
#define            MUSIC_IN_TDM0  0x0000
#define            MUSIC_IN_TDM1  0x0100
#define            MUSIC_IN_TDM2  0x0200
#define            MUSIC_IN_TDM3  0x0300
#define            USE_TDM_MUSIC_TO_SYNC  0x0400
#define            TDM_SYNC_LEFT_CH  0x0000
#define            TDM_SYNC_RIGHT_CH  0x0800

#define            TDM_SYNC_DELAY_1_CLKS_CYCLES 0x1000
#define            TDM_SYNC_DELAY_2_CLKS_CYCLES 0x2000
#define            TDM_SYNC_DELAY_3_CLKS_CYCLES 0x3000
#define            TDM_SYNC_DELAY_4_CLKS_CYCLES 0x4000
#define            TDM_SYNC_DELAY_5_CLKS_CYCLES 0x5000
#define            TDM_SYNC_DELAY_6_CLKS_CYCLES 0x6000
#define            TDM_SYNC_DELAY_7_CLKS_CYCLES 0x7000
#define            TDM_SYNC_DELAY_8_CLKS_CYCLES 0x8000
#define            TDM_SYNC_DELAY_9_CLKS_CYCLES 0x9000
#define            TDM_SYNC_DELAY_10_CLKS_CYCLES 0xA000
#define            TDM_SYNC_DELAY_11_CLKS_CYCLES 0xB000
#define            TDM_SYNC_DELAY_12_CLKS_CYCLES 0xC000
#define            TDM_SYNC_DELAY_13_CLKS_CYCLES 0xD000
#define            TDM_SYNC_DELAY_14_CLKS_CYCLES 0xE000
#define            TDM_SYNC_DELAY_15_CLKS_CYCLES 0xF000
                               


#define READ_AUDIO_BUFFER  0x20
#define GENERAL_CONFIG_1  0x22
#define            MAX_NUMBER_OF_MIC_IS_0  0x0000
#define            MAX_NUMBER_OF_MIC_IS_2  0x0001
#define            MAX_NUMBER_OF_MIC_IS_4  0x0002
#define            DSP_CLK_GEN_PLL  0x0000
#define            DSP_CLK_GEN_RC_OSC  0x0020
#define            IF_REQUIRED_WAKEUP  0x0040
#define            AUD_BUF_READ_MODE_MODE_2  0x0000
#define            AUD_BUF_READ_MODE_MODE_1_AND_2  0x0080
#define            WAKEUP_POLARITY_ACTIVE_LOW  0x0000
#define            WAKEUP_POLARITY_ACTIVE_HIGH  0x0100
#define            MEMORY_SLEEP_EN  0x0200
#define            INVERT_DETECTION_GPIOS_ACTIVE_LOW   0x0400
#define            INVERT_DETECTION_GPIOS_ACTIVE_HIGH   0x0000
#define            SEND_UART_CHG_SEQ  0x0800
#define            SPLIT_MIC_BUFFER  0x1000

#define GENERAL_CONFIG_2  0x23
#define            FW_VAD_TYPE_NO_VAD  0x0000
#define            FW_VAD_TYPE_HW_VAD0  0x0020
#define            FW_VAD_TYPE_FW_OR_VESPER  0x0040
#define            FW_VAD_TYPE_D6_HW_VAD1  0x0060
#define            DDF_SAMPLE_WIDTH_16_BIT  0x0000
#define            DDF_SAMPLE_WIDTH_24_BIT  0x0080
#define            MIC_SAMPLE_RATE_16K  0x0000
#define            MIC_SAMPLE_RATE_44_1K  0x0100
#define            MIC_SAMPLE_RATE_48K  0x0200
#define            MIC_SAMPLE_RATE_8K  0x0300
#define            AUTO_MIPS_HWVAD_ENABLED  0x0400
#define            RESET_DDF_DC  0x0800
#define            WRITE_AUD_BUF_HDR  0x1000
#define            D6_LDO_SETTING_INPUT_MIN_0_96V  0x2000
#define            D6_LDO_SETTING_INPUT_MIN_1_03V  0x4000
#define            D6_LDO_SETTING_INPUT_MIN_1_1V  0x6000

#define FIRST_MICROPHONE_CONFIG  0x24
#define SECOND_MICROPHONE_CONFIG  0x25
#define THIRD_MICROPHONE_CONFIG  0x26
#define FOURTH_MICROPHONE_CONFIG  0x27
#define            D2_MIC_SRC_LEFT_DMIC_DM0   0x1
#define            D2_MIC_SRC_RIGHT_DMIC_DM0   0x2
#define            D2_MIC_SRC_LEFT_DMIC_DM1   0x3
#define            D2_MIC_SRC_RIGHT_DMIC_DM1   0x4
#define            D2_MIC_SRC_ANALOG_MIC_DM2   0x5
#define            D2_DM_DATA_SRC_DM0_TDM2_RX   0x0
#define            D2_DM_DATA_SRC_DM1_TDM0_FSYNC   0x0
#define            D2_DM_DATA_SRC_DM2_HIGH_IMPEDANCE   0x0
#define            D2_DM_DATA_SRC_DM0_TDM3_FSYNC   0x8
#define            D2_DM_DATA_SRC_DM1_TDM3_RX   0x8
#define            D2_DM_DATA_SRC_DM2_MID_IMPEDANCE   0x8
#define            D2_DM_CLK_SRC_DM0_TDM2_TX   0x0
#define            D2_DM_CLK_SRC_DM1_TDM0_TX   0x0
#define            D2_DM_CLK_SRC_DM0_TDM3_SCLK   0x10
#define            D2_DM_CLK_SRC_DM1_TDM3_TX   0x10

#define            DDF_AND_DM_CONFIG_DDF0_DM0   0x1
#define            DDF_AND_DM_CONFIG_DDF0_DM1   0x2
#define            DDF_AND_DM_CONFIG_DDF1_DM0   0x3
#define            DDF_AND_DM_CONFIG_DDF1_DM1   0x4
#define            DDF_AND_DM_CONFIG_SD_DDF_DM0   0x5
#define            DDF_AND_DM_CONFIG_SD_DDF_DM1   0x6
#define            DDF_AND_DM_CONFIG_SD_DDF_SD_ADC   0x7
#define            DDF_AND_DM_CONFIG_SAR_DDF_SAR_ADC   0x8
#define            D10L_DDF_AND_DM_CONFIG_DDF0_DM0   0x1
#define            D10L_DDF_AND_DM_CONFIG_DDF1_DM0   0x2
#define            D10L_DDF_AND_DM_CONFIG_DDF2_DM1   0x3
#define            D10L_DDF_AND_DM_CONFIG_DDF3_DM1   0x4
//RSVD1					     0x0005
//RSVD1					     0x0006
//RSVD1					     0x0007
#define            D10L_DDF_AND_DM_CONFIG_SAR0_D10L_DDF_SAR_ADC   0x8
#define            D10L_DDF_AND_DM_CONFIG_SAR1_D10L_DDF_SAR_ADC   0x9
#define            CLOCK_POLARITY_RISING_EDGE   0x0
#define            CLOCK_POLARITY_FALLING_EDGE   0x10
          
#define            DM_CLK_FREQ_512_SR_8KHz_16KHz   0x0
#define            DM_CLK_FREQ_768_SR_8KHz_16KHz   0x20
#define            DM_CLK_FREQ_1024_1040_SR_16KHz_32KHz_48KHz   0x40
#define            DM_CLK_FREQ_1536_1200_SR_8KHz_16KHz_32KHz_48KHz   0x60
#define            DM_CLK_FREQ_2304_2352_SR_16KHz_32KHz_48KHz   0x80
#define            DM_CLK_FREQ_3072_SR_16KHz_32KHz_48KHz   0xa0
#define            SAR_IIR_FILTER_128   0x0
#define            SAR_IIR_FILTER_256   0x20
#define            SAR_IIR_FILTER_512   0x60
#define            SAR_IIR_FILTER_1024   0x80
#define            SAR_IIR_FILTER_SD_ALIGN_D8D   0x40
#define            SAR_IIR_FILTER_64_FOR_TDM0_44_1   0x80
#define            SYNCED_START   0x100
#define            DDF_AUDIO_ATTENUATION_6dB_GAIN   0x0
#define            DDF_AUDIO_ATTENUATION_0dB   0x200
#define            DDF_AUDIO_ATTENUATION_MINUS_6dB   0x400
#define            DDF_AUDIO_ATTENUATION_MINUS_12dB   0x600
#define            SAR_ADC_SEL_MAIN   0x0
#define            SAR_ADC_SEL_SEC   0x800          
#define            DM_DATA_SRC_DM0_GPIO2_DM1_GPIO1   0x0
#define            DM_DATA_SRC_DM0_GPIO5_DM1_GPIO11   0x1000
#define            DM_DATA_SRC_DM0_GPIO9_DM1_GPIO13   0x2000
#define            DM_DATA_SRC_DM0_GPIO20_DM1_GPIO18   0x3000
#define            DM_CLOCK_SRC_DM0_GPIO6_DM1_GPIO0   0x0
#define            DM_CLOCK_SRC_DM0_GPIO6_DM1_GPIO12   0x4000
#define            DM_CLOCK_SRC_DM0_GPIO8_DM1_GPIO14   0x8000
#define            DM_CLOCK_SRC_DM0_GPIO19_DM1_GPIO17   0xC000
          
#define            CLOSING_MICS_NO_DM_CLOCK   0x200
#define            CLOSING_MICS_SAR_LOW_AMP   0x0


#define VESPER_MIC_CONFIG  0x28
#define            MODE_GPIO_NUM_0 0x0
#define            MODE_GPIO_NUM_1 0x1
#define            MODE_GPIO_NUM_2 0x2
#define            MODE_GPIO_NUM_3 0x3
#define            MODE_GPIO_NUM_4 0x4
#define            MODE_GPIO_NUM_5 0x5
#define            MODE_GPIO_NUM_6 0x6
#define            MODE_GPIO_NUM_7 0x7
#define            MODE_GPIO_NUM_8 0x8
#define            MODE_GPIO_NUM_9 0x9
#define            MODE_GPIO_NUM_10 0xa
#define            MODE_GPIO_NUM_11 0xb
#define            MODE_GPIO_NUM_12 0xc
#define            MODE_GPIO_NUM_13 0xd
#define            MODE_GPIO_NUM_14 0xe
#define            MODE_GPIO_NUM_15 0xf
#define            MODE_GPIO_NUM_16 0x10
#define            MODE_GPIO_NUM_17 0x11
#define            MODE_GPIO_NUM_18 0x12
#define            MODE_GPIO_NUM_19 0x13
#define            MODE_GPIO_NUM_20 0x14
#define            MODE_GPIO_NUM_21 0x15
#define            MODE_GPIO_NUM_22 0x16
#define            MODE_GPIO_NUM_23 0x17
#define            MODE_GPIO_NUM_24 0x18
                   
#define            DET_GPIO_NUM_0 0x0
#define            DET_GPIO_NUM_1 0x20
#define            DET_GPIO_NUM_2 0x40
#define            DET_GPIO_NUM_3 0x60
#define            DET_GPIO_NUM_4 0x80
#define            DET_GPIO_NUM_5 0xa0
#define            DET_GPIO_NUM_6 0xc0
#define            DET_GPIO_NUM_7 0xe0
#define            DET_GPIO_NUM_8 0x100
#define            DET_GPIO_NUM_9 0x120
#define            DET_GPIO_NUM_10 0x140
#define            DET_GPIO_NUM_11 0x160
#define            DET_GPIO_NUM_12 0x180
#define            DET_GPIO_NUM_13 0x1a0
#define            DET_GPIO_NUM_14 0x1c0
#define            DET_GPIO_NUM_15 0x1e0
#define            DET_GPIO_NUM_16 0x200
#define            DET_GPIO_NUM_17 0x220
#define            DET_GPIO_NUM_18 0x240
#define            DET_GPIO_NUM_19 0x260
#define            DET_GPIO_NUM_20 0x280
#define            DET_GPIO_NUM_21 0x2a0
#define            DET_GPIO_NUM_22 0x2c0
#define            DET_GPIO_NUM_23 0x2e0
#define            DET_GPIO_NUM_24 0x300
      
#define            SIL_TIME_THD_250MS 0x1000
#define            SIL_TIME_THD_500MS 0x2000
#define            SIL_TIME_THD_1000MS 0x4000
#define            SIL_TIME_THD_2000MS 0x8000
#define            SIL_TIME_THD_4000MS 0xf000

// VAD_MIC_SILENCE_CONFIG_1_341
#define SIL_LOW_THD 			SILENCE_LOW_THD

// VAD_MIC_SILENCE_CONFIG_2_342
#define SIL_HIGH_THD 			SILENCE_HIGH_THD

//VAD_MIC_ACTIVE_PERIOD_CONFIG_343
#define LONG_ACTIVE_THD			LONG_ACTIVE_PERIOD<<8 	     
#define SHORT_ACTIVE_THD		SHORT_ACTIVE_PERIOD

//VAD_MIC_RTC_CONFIG_1_344
#define MID_RATE_FACTOR			(RTC_MID_RATE_FACTOR-1)<<12   
#define LOW_RATE_FACTOR			(RTC_LOW_RATE_FACTOR-16)<<6  
#define RTC_PERIOD_BASE			(RTC_PERIOD_BASE_NUM-20)		

//VAD_MIC_RTC_CONFIG_2_345
#define ADAPT_RATE_HIGH_THD		(RTC_ADAPT_RATE_HIGH_THD-16)<<4  
#define ADAPT_RATE_LOW_THD		(RTC_ADAPT_RATE_LOW_THD-16)   	

//VAD_MIC_MAX_GAIN_CONFIG_346
#define PGA_MAX_STEP_DOWN 		PGA_MAX_STEP_DOWN_DEC<<8    	
#define PGA_MAX_HIGH_THD  		(PGA_MAX_HIGH_BAUNDRY-16)<<4  
#define PGA_MAX_LOW_THD	  		(PGA_MAX_LOW_BAUNDRY-16) 	

// SENSOR_EVENT_401
#define INVN_ALGO_AML_SWIPE_LEFT                0x1
#define INVN_ALGO_AML_SWIPE_RIGTH               0x2
#define INVN_ALGO_AML_SWIPE_UP                  0x4
#define INVN_ALGO_AML_SWIPE_DOWN                0x8
#define INVN_ALGO_AML_SWIPE_CLOCKWISE           0x10
#define INVN_ALGO_AML_SWIPE_COUNTERCLOCKWISE    0x20

// SENSOR_EN_DIS_404

#define START_IMU_PROCESS 			0x800
#define STOP_IMU_PROCESS 			0x00

// VT SED engine fields
#define VT_FE_THUD_ENABLE 			0x3
#define VT_FE_THUD_REPORTING_ENABLE 0x4

#define VT_DNN_STEP_SIZE 			0x23
#define VT_DNN_INPUT_PATCH_SIZE     0x33 //Not in use

#define VT_PP_NUM_OF_CLASSES 		0x12
#define VT_PP_CLASSES_TO_REPORT     0x13 
#define VT_PP_IIR_MODE 			    0x14 //Not in use
#define VT_FE_DNN_PATCH_SIZE		0x33

#define FE_THUD_ENABLE  			0
#define FE_THUD_REPORTING_ENABLE 	0

#define DNN_STEP_SIZE_GB_SW	23
#define DNN_STEP_SIZE_TF_SW 23
#define DNN_STEP_SIZE_BC_SW 23
#define DNN_STEP_SIZE_GB_HW 12
#define DNN_STEP_SIZE_TF_HW 12
#define DNN_STEP_SIZE_BC_HW 12
#define DNN_STEP_SIZE_HW	6

#define PP_NUM_OF_CLASSES   	0x2
#define PP_CLASSES_TO_REPORT 	0x1
#define PP_DNN_OUT_SMOOTH  		0


                               
#define HOST_INTERFACE_SUPPORT  0x29
#define            UART0_ENABLE_FOR_HOST_D2_TDM2_D4_D6_GPIO_17_18_D10_GPIO_2_5  0x0001
#define            I2C_D2_ON_I2C_SDA_SCK_D4_D6_GPIO_1_2_D10_GPIO_3_4  0x0002
#define            SLIMBUS_D2_TDM0_D4_D6_GPIO_13_15  0x0004
#define            UART_D2_0_UART_TDI_RTCK_D4_1_GPIO_14_15_D10_GPIO_12_13  0x0008
#define            UART_D2_0_UART_Rx_Tx_D4_1_GPIO_8_9  0x0010
#define            SPI_D2_TDM1_D4_GPIO_4_5_6_7_D10_GPIO_2_3_4_5  0x0020
#define            SPI_D2_SPI_PINS_D6_GPIO_4_5_6_7  0x0040
#define            UART_D2_0_ON_I2C_SDA_SCK  0x0080
#define            SPI_ACK  0x0800
#define            UART_DEBUG_MODE_0  0x1000
#define            UART_DEBUG_MODE_1  0x2000
#define            UART_EN_UART_1_Rx  0x4000

#define TONE_GENERATOR  0x2A
#define            TONE_GEN_EN  0x0001
#define            TONE_WORD_WIDTH_16_BITS  0x0000
#define            TONE_WORD_WIDTH_32_BITS  0x0002

#define LDO_CONFIG  0x2B
#define            LDO_DIS  0x0
#define            LDO_EN  0x1
#define            LDO_CAS_OUT_LEVEL_1_2_AND_BELLOW  0x2
#define            LDO_CAS_OUT_LEVEL_1_8  0x4
#define            LDO_EN_6PCNT  0x200
#define            LDO_EN_2PCNT  0x400

#define VT_COMMAND_TIMEOUT  0x2C
#define            TIMEOUT_3_SEC  0xBB8
#define            TIMEOUT_5_MIN  0x4e20


#define SPL_METER_CONFIG  0x2D
#define SPL_METER_STATUS  0x2E

#define UART_DEBUG_RECORDING  0x30
#define            SET_CLR_CONFIG  0x8000
#define            SET_ACTIVATION_STATUS  0x0
#define            ENABLE_RECORDING  0x1
#define            DISABLE_RECORDING  0x0
#define            CLEAR_REC_BUFFER  0x400
#define            ENLARGE_BUFFER_BY_1_5  0x2
#define            ENLARGE_BUFFER_BY_2  0x4
#define            ALLOCATE_BUFFER_IN_AHB_MEMORY  0x8
#define            ADD_BUFFER_TO_HW_VAD  0x10
#define            ENABLE_CALCULATING_MAX_VALUES  0x100
#define            PRINT_MAX_VALUES  0x200
#define            RECORD_MIC0_INT  0x1
#define            RECORD_MIC1_INT  0x2
#define            RECORD_MIC2_INT  0x4
#define            RECORD_MIC3_INT  0x8
#define            RECORD_MIC0_FRM  0x10
#define            RECORD_MIC1_FRM  0x20
#define            RECORD_MIC2_FRM  0x40
#define            RECORD_MIC3_FRM  0x80
#define            RECORD_TDM0_Rx_INT  0x100
#define            RECORD_TDM1_Rx_INT  0x200
#define            RECORD_TDM2_Rx_INT  0x400
#define            RECORD_TDM3_Rx_INT  0x800
#define            RECORD_TDM0_Tx_INT  0x1000
#define            RECORD_TDM1_Tx_INT  0x2000
#define            RECORD_TDM2_Tx_INT  0x4000
#define            RECORD_TDM3_Tx_INT  0x8000
#define            RECORD_TDM0_Rx_FRM  0x1
#define            RECORD_TDM1_Rx_FRM  0x2
#define            RECORD_TDM2_Rx_FRM  0x4
#define            RECORD_TDM3_Rx_FRM  0x8
#define            RECORD_TDM0_Tx_FRM  0x10
#define            RECORD_TDM1_Tx_FRM  0x20
#define            RECORD_TDM2_Tx_FRM  0x40
#define            RECORD_TDM3_Tx_FRM  0x80
#define            RECORD_BUF1_IN_FRM  0x100
#define            RECORD_BUF2_IN_FRM  0x200
#define            RECORD_BUF3_IN_FRM  0x400
#define            RECORD_BUF4_IN_FRM  0x800
#define            RECORD_BUF1_OUT_FRM  0x1000
#define            RECORD_BUF2_OUT_FRM  0x2000
#define            RECORD_BUF3_OUT_FRM  0x4000
#define            RECORD_BUF4_OUT_FRM  0x8000
#define            RECORD_VT_IN  0x1
#define            RECORD_BUF_8_OUT  0x2
#define            RECORD_BUF_9_OUT  0x4
#define            RECORD_DECIM_IN_0  0x8
#define            RECORD_DECIM_IN_1  0x10


#define TDM_ACTIVATION_CONTROL  0x31
#define            CONFIG_TDM_0   0x0
#define            CONFIG_TDM_1   0x1
#define            CONFIG_TDM_2   0x2
#define            CONFIG_TDM_3   0x3
#define            PROC_EN_DISABLE   0x0
#define            PROC_EN_SWITCH_FW_TO_DETECTION_MODE   0x4
#define            PROC_EN_SWITCH_FW_TO_BUFFERIG_MODE   0x8
#define            TDM0_RX_EN   0x100
#define            TDM0_TX_EN   0x200
#define            TDM1_RX_EN   0x400
#define            TDM1_TX_EN   0x800
#define            TDM2_RX_EN   0x1000
#define            TDM2_TX_EN   0x2000
#define            TDM3_RX_EN   0x4000
#define            TDM3_TX_EN   0x8000


#define MEMORY_CONFIG  0x33
#define            DTCM_SIZE_64_KW  0x0000
#define            DTCM_SIZE_D4_D6_96_KW_D8_128KW  0x0001
#define            DTCM_SIZE_D4_D6_96_KW_D8_192KW  0x0002
#define            AUDIO_BUF_LOC_DTCM_USING_MEM_ALLOCATION  0x0000
#define            AUDIO_BUF_LOC_AHB_USING_MEM_ALLOCATION  0x0004
#define            AUDIO_BUF_LOC_AHB_3_SEC_BUFFER  0x0008
#define            AUDIO_BUF_LOC_AHB_7_SEC_BUFFER_D8  0x000C
#define            AHB_ON_SIZE_NO_AHB  0x0000
#define            AHB_ON_SIZE_32KW  0x0010
#define            AHB_ON_SIZE_64KW_32KW_CACHE  0x0020
#define            AHB_ON_SIZE_80KW_16KW_CACHE  0x0030
#define            AHB_ON_SIZE_112KW_32KW_CACHE  0x0040
#define            AHB_ON_SIZE_128KW_16KW_CACHE  0x0050
#define            AHB_ON_SIZE_160KW_32KW_CACHE_D8  0x0060
#define            AHB_ON_SIZE_192KW_32KW_CACHE_D8  0x0070
#define            AMODEL_0_LOC_DTCM  0x0000
#define            AMODEL_1_LOC_DTCM  0x0000
#define            AMODEL_2_LOC_DTCM  0x0000
#define            AMODEL_3_LOC_DTCM  0x0000
#define            AMODEL_4_LOC_DTCM  0x0000
#define            AMODEL_0_LOC_AHB  0x0100
#define            AMODEL_1_LOC_AHB  0x0200
#define            AMODEL_2_LOC_AHB  0x0400
#define            AMODEL_3_LOC_AHB  0x0800
#define            AMODEL_4_LOC_AHB  0x1000
#define                        MIC_AUDIO_BUF_LOC_DTCM  0x0000
#define                        MIC_AUDIO_BUF_LOC_AHB  0x2000

#define AUDIO_PROCESSING_CONFIG  0x34
#define            ALGO1_EN_FW_MODE_1_ONLY  0x0001
#define            ALGO1_EN_FW_MODE_2_ONLY  0x0002
#define            ALGO1_EN_FW_MODE_1_AND_MODE_2  0x0003
#define            ALGO2_EN_FW_MODE_1_ONLY  0x0004
#define            ALGO2_EN_FW_MODE_2_ONLY  0x0008
#define            ALGO2_EN_FW_MODE_1_AND_MODE_2  0x000C
#define            ALGO3_EN_FW_MODE_1_ONLY  0x0010
#define            ALGO3_EN_FW_MODE_2_ONLY  0x0020
#define            ALGO3_EN_FW_MODE_1_AND_MODE_2  0x0030
#define            VT_PROC_EN  0x0040
#define            EN_441_EC_REF_DECIM  0x0080
#define            VT_CP0  0x0000
#define            VT_CP1  0x0100
#define            VT_CP2  0x0200
#define            VT_CP3  0x0300
#define            VT_CP4  0x0400
#define            VT_CP5  0x0500
#define            VT_CP6  0x0600
#define            VT_CP7  0x0700
#define            AUTO_SET_HIGH_MIPS  0x1000
#define            POST_DETECTION_MODE_SWITCH_TO_STREAMING  0x0000
#define            POST_DETECTION_MODE_REMAIN_IN_DETECTION  0x2000
#define            EN_48_EC_REF_DECIM  0x4000
#define            EN_COPY_HIST  0x8000

#define TDM_RX_CONFIG  0x36
#define TDM_TX_CONFIG  0x37
#define            CLEAR_TDM_CFG  0x0
#define            RX_TX_CP0  0x0000
#define            RX_TX_CP1  0x0001
#define            RX_TX_CP2  0x0002
#define            RX_TX_CP3  0x0003
#define            RX_TX_CP4  0x0004
#define            RX_TX_CP5  0x0005
#define            RX_TX_CP6  0x0006
#define            RX_TX_CP7  0x0007
#define            RX_TX_CP8  0x0008
#define            RX_TX_CP9  0x0009
#define            RX_TX_CP_VT_AUDIO_BUFFER  0x000f
#define            HW_BLOCK_EN  0x0010
#define            RESAMPLE_RATIO_NO_RESAMPLING  0x0000
#define            RESAMPLE_RATIO_2_AND_1_OR_44_1  0x0020
#define            RESAMPLE_RATIO_3_1  0x0040
#define            RESAMPLE_TYPE_DECIMATION  0x0000
#define            RESAMPLE_TYPE_INTERPOLATION  0x0080
#define            RX_TX_I2S_CH_USE_I2S_STEREO  0x0000
#define            TX_I2S_CH_SUM_HIGH_AND_LOW_INTO_ONE_SAMPLE  0x0100
#define            TX_MASTER_CLOCK_ONLY  0x0200
#define            TX_FULL_OPERATION  0x0
#define            INPUT_OUTPUT_SAMPLE_RATE_16_KHz  0x0000
#define            INPUT_OUTPUT_SAMPLE_RATE_44_1_KHz  0x0400
#define            INPUT_OUTPUT_SAMPLE_RATE_48_KHz  0x0800
#define            INPUT_OUTPUT_SAMPLE_RATE_8_KHz  0x0C00
#define            NUM_OF_CHANNELS_1_CH  0x0000
#define            NUM_OF_CHANNELS_2_CH  0x1000
#define            NUM_OF_CHANNELS_4_CH  0x2000
#define            SAMPLE_WIDTH_16_BIT  0x0000
#define            SAMPLE_WIDTH_32_BIT  0x4000
#define            DEMUX_MUX_DISABLE  0x0000
#define            DEMUX_MUX_ENABLE  0x8000

#define BUFFERING_NORMAL_AMPLITUDE 0x38
#define NORMALIZE_TO_MINUS_1dB 0x3f
#define NORMALIZE_TO_MINUS_3dB 0x3d
#define NORMALIZE_TO_MINUS_6dB 0x3a
#define NORMALIZE_TO_MINUS_9dB 0x37
#define NORMALIZE_TO_MINUS_18dB 0x2e
#define NO_NORMALIZATION 0x0
#define USE_PHRASE_LEN_CONFIG_BY_HOST 0x80
#define USE_PHRASE_LEN_FROM_WWE 0x0
#define USER_CONFIG_PHRASE_LEN_500_MS 0x3200
#define USER_CONFIG_PHRASE_LEN_600_MS 0x3C00
#define USER_CONFIG_PHRASE_LEN_700_MS 0x4600


#define INDIRECT_ACCESS_REGISTER_NUMBER  0x3D
#define INDIRECT_ACCESS_REGISTER_WRITE  0x3E
#define INDIRECT_ACCESS_REGISTER_READ  0x3F


/* VT Engine  commands registers */
#define VT_ENGINE_RECOGMODE 0x40
#define VT_ENGINE_INITIALIZED 0x41
#define VT_ENGINE_LPSD_VAD 0x42
#define VT_ENGINE_DUALTHRESHOLD 0x45
#define VT_ENGINE_WORDID 0x5B
#define VT_ENGINE_PHRASE_LENGTH 0x67

/* VT Engine Registers */
#define VT_NUM_GLOBAL 0x0
#define VT_NUM_1 0x2000
#define VT_NUM_2 0x4000
#define VT_NUM_3 0x6000
#define VT_NUM_NONE 0xE000

#define VT_MAPPING  0x300
#define VT_FRAME_SIZE  0x301


#define VT_GENERAL_CFG 0x304
/*-------------------*/
#define VT_EN_DIS  0x0
#define VT_EN_EN  0x1000
#define VT_ACTIVE  0x800
#define VT_PAUSE  0x0
#define SET_IN1_CP  0x200
#define IN1_CP_NUM_0  0x0
#define IN1_CP_NUM_1  0x20
#define IN1_CP_NUM_2  0x30
#define IN1_CP_NUM_3  0x40
#define IN1_CP_NUM_4  0x50
#define IN1_CP_NUM_5  0x60
#define IN1_CP_NUM_6  0x70
#define IN1_CP_NUM_7  0x80
#define SET_IN0_CP  0x10
#define IN0_CP_NUM_0  0x0
#define IN0_CP_NUM_1  0x1
#define IN0_CP_NUM_2  0x2
#define IN0_CP_NUM_3  0x3
#define IN0_CP_NUM_4  0x4
#define IN0_CP_NUM_5  0x5
#define IN0_CP_NUM_6  0x6
#define IN0_CP_NUM_7  0x7


#define VT_AUDIO_HISTORY_CFG 0x305
/*-------------------*/
#define START_BUF_READ 0x1000
#define EXT_HIST_TIME_0MS 0x0
#define EXT_HIST_TIME_500MS 0x1F4
#define EXT_HIST_TIME_750MS 0x5DC
#define EXT_HIST_TIME_1S 0x3E8
#define HIST_POINT_IS_SWITCH_TO_BUFFER 0x0
#define HIST_POINT_IS_WW_END 0x1
#define HIST_POINT_IS_WW_START 0x2

#define VT__RECORD_CFG 0x306
/*-------------------*/
#define OUT_SIZE_SAME 0x0
#define OUT_SIZE_EXPANDED 0x200
#define MARK_DET 0x100
#define REC_OUT_DATA 0x10
#define REC_IN_DATA 0x1

#define VT1_REGS_OFFSET 0x40
#define VT2_REGS_OFFSET 0x80
#define VT3_REGS_OFFSET 0xC0

#define VT_REG_ACTIVATE_MODEL_TYPE  0x2A
#define VT_REG_ACTIVATE_NUM_GROUPS  0x2B
#define VT_REG_SET_LEVEL_2_TIMEOUT	0x2C

#define VT_ACTIVATE_MODEL_DSP       0x0
#define VT_ACTIVATE_MODEL_NNL       0x1

#define VT_ACTIVATE_GROUP_1         0x1
#define VT_ACTIVATE_GROUP_1_AND_2   0x2

#define VT_REG_MODEL_SELECT 		0x13
#define TRIGGER_ON_PRIMERY_MODEL 	0x0

#define DTE_PARAM_ACTIVE_AMODEL		0x9
#define ENABLE_COMMAND_TO_TRIGGER_SWITCH 0x1


#define VT_REG_RECOG_MODE 0x0
#define VT_RECOG_MODE_SINGLE_INSTANCE 0x1
#define VT_RECOG_MODE_LPSD_VAD 0x2
#define VT_RECOG_MODE_DUAL_INSTANCES 0x3
#define VT_INST_EN_INST_1 0x1000
#define VT_INST_EN_INST_2 0x2000
#define VT_INST_EN_INST_3 0x4000
#define VT_INST_EN_INST_4 0x8000
#define VT_INST_ACTIVE_PAUSE_INST_1 0x100
#define VT_INST_ACTIVE_PAUSE_INST_2 0x200
#define VT_INST_ACTIVE_PAUSE_INST_3 0x400
#define VT_INST_ACTIVE_PAUSE_INST_4 0x800
#define VT_REG_INIT 0x1
#define VT_REG_TH 0x8
#define VT_REG_LL 0xA
#define VT_REG_WORD_ID 0x1B
#define VT_FIRST_INST_DET 0x4000
#define VT_SECOND_INST_DET 0x8000
#define VT_REG_PHRASE_LENGTH 0x27
#define VT_REG_DETECTION_DELAY 0x16
#define VT_REG_ACTIVATE_MODEL_TYPE 0x2A
#define VT_REG_ACTIVATE_NUM_GROUPS 0x2B

#define VT_ACTIVATE_MODEL_DSP 0x0
#define VT_ACTIVATE_MODEL_NNL 0x1

#define VT_ACTIVATE_GROUP_1 0x1
#define VT_ACTIVATE_GROUP_1_AND_2 0x2

#define USER_CFG_WWE_LENGTH_500_MS 0x2000
#define USER_CFG_WWE_LENGTH_750_MS 0x3000
#define USER_CFG_WWE_LENGTH_1S 0x4000

#define ASRP_LIB_VER  0x100
#define ASRP_TUNING_VER  0x101
#define ASRP_NUM_OF_ERRS  0x104
#define ASRP_USER_DEFINED_DELAY  0x107
#define ASRP_OUTPUT_ROUTING  0x109
#define ASRP_OUTPUT_SRC_BFPF_0  0x0
#define ASRP_OUTPUT_SRC_BFPF_1  0x100
#define ASRP_OUTPUT_SRC_VCPF_1  0x200
#define ASRP_OUTPUT_SRC_BF_1  0x300
#define ASRP_OUTPUT_SRC_AEC_1  0x400
#define ASRP_OUTPUT_SRC_AEC_2  0x500
#define ASRP_OUTPUT_SRC_AEC_3  0x600
#define ASRP_OUTPUT_DEST_TX_1  0x0
#define ASRP_OUTPUT_DEST_TX_2  0x1
#define ASRP_OUTPUT_DEST_TX_3  0x2
#define ASRP_OUTPUT_DEST_TX_4  0x3
#define ASRP_OUTPUT_DEST_TX_5  0x4
#define ASRP_OUTPUT_DEST_TX_6  0x5
#define ASRP_OUTPUT_DEST_TX_7  0x6
#define ASRP_OUTPUT_DEST_TX_8  0x7
#define ASRP_BLOCKID_LOW_PART  0x124
#define ASRP_BLOCKID_HIGH_PART  0x125
#define ASRP_ENABLE_OFFSET  0x126
#define ASRP_VALUE  0x127
#define ASRP_RECORDS_CHANNELS  0x128
#define ASRP_REC_CH_TX_IN_1  0x1
#define ASRP_REC_CH_TX_IN_2  0x2
#define ASRP_REC_CH_TX_IN_3  0x4
#define ASRP_REC_CH_TX_IN_4  0x8
#define ASRP_REC_CH_RX_IN_1  0x10
#define ASRP_REC_CH_RX_IN_2  0x20
#define ASRP_REC_CH_RX_OUT_1  0x40
#define ASRP_REC_CH_TX_PROC_TX_IN_1  0x100
#define ASRP_REC_CH_TX_PROC_TX_IN_2  0x200
#define ASRP_REC_CH_TX_PROC_TX_IN_3  0x400
#define ASRP_REC_CH_TX_PROC_RX_IN_1  0x1000
#define ASRP_REC_CH_TX_PROC_RX_IN_2  0x2000
#define ASRP_REC_CH_TX_PROC_RX_OUT_1  0x4000
#define ASRP_RECORDS_CHANNELS_2  0x129
#define ASRP_REC_CH_TX_OUT_1  0x1
#define ASRP_REC_CH_TX_OUT_2  0x2
#define ASRP_REC_CH_TX_OUT_3  0x4
#define ASRP_QEPD_TRIGGER_MODE  0x140
#define ASRP_QEPD_TRIGGER_OFFSET_MODE  0x141
#define ASRP_QEPD_TRIGGER_LENGTH  0x142
#define ASRP_QEPD_OUT_ATTENUATION  0x143
#define ASRP_QEPD_OUT_DELAY  0x144
#define ASRP_QEPD_OUT_GAIN  0x145
#define ASRP_QED_END	0x147
#define AEP_STATUS_REG  0x10c

#define ASRP_OUTPUT_GAIN_DEST_SEL  0x10A
#define ASRP_TX_OUT  0x0
#define ASRP_VCPF_OUT  0x1
#define ASRP_RX_OUT  0x2
#define ASRP_OUTPUT_GAIN_DEST_VALUE  0x10B
#define ASRP_FULL_BYPASS  0x1
#define ASRP_SEND_QED_MARK  0x1


// Hardware TDM registers
#define D4_TDM_0_RX_ADDR   0x80800000
#define D4_TDM_1_RX_ADDR   0x80801000
#define D4_TDM_0_TX_ADDR   0x80804000
#define D4_TDM_1_TX_ADDR   0x80805000

// Hardware TDM registers
#define D10_TDM_0_RX_ADDR   0x90000000
#define D10_TDM_1_RX_ADDR   0x90800000
#define D10_TDM_2_RX_ADDR   0x95000000
#define D10_TDM_0_TX_ADDR   0x90000400
#define D10_TDM_1_TX_ADDR   0x90800400
#define D10_TDM_2_TX_ADDR   0x95000400

// Hardware TDM registers
#define D2_TDM_0_RX_ADDR   0x80800000
#define D2_TDM_1_RX_ADDR   0x80801000
#define D2_TDM_2_RX_ADDR   0x80802000
#define D2_TDM_3_RX_ADDR   0x80803000
#define D2_TDM_0_TX_ADDR   0x80804000
#define D2_TDM_1_TX_ADDR   0x80805000
#define D2_TDM_2_TX_ADDR   0x80806000
#define D2_TDM_3_TX_ADDR   0x80807000

// Hardware IOM registers
#define IOM1   0x300004C
#define IOM2   0x3000050
#define GP_DATA_SET   0x4000004
#define GP_DATA_CLR   0x4000008
#define GP_DIR_OUT   0x4000010
#define CLK_DIV_1   0x300000C
#define AHB_PMOD   0x3000020

// Hardware IO registers
#define IOKE_ADDRESS   0x3000060
#define IOPE_ADDRESS   0x3000064
#define IOPV_ADDRESS   0x3000068

#define IOKE_VALUE   0xfdffff
#define IOPE_VALUE   0x20000
#define IOPV_VALUE   0x6007c6

#define MASK_UART_RX   0x20000
#define MASK_IO_PULL_DOWN   0x1DE4F1

#define BOOT_PRE_OP_CODE   [0x5A]
#define BOOT_LOAD_PAYLOAD_TO_PROG_MEM   [0x01]
#define BOOT_LOAD_PAYLOAD_TO_DATA_MEM   [0x02]
#define BOOT_WRITE_INT32_TO_AHB_ADDR   [0x03]
#define BOOT_WRITE_INT32_TO_APB_ADDR   [0x04]
#define BOOT_WRITE_INT32_TO_CPM_ADDR   [0x05]
#define BOOT_READ_INT32_FROM_AHB_ADDR   [0x06]
#define BOOT_READ_INT32_FROM_APB_ADDR   [0x07]
#define BOOT_READ_INT32_FROM_CPM_ADDR   [0x08]
#define BOOT_SET_MASK_VAL_TO_APB_REG   [0x09]
#define BOOT_CALL_FUNC_ADDR   [0x0A]
#define BOOT_RUN_FW   [0x0B]
#define BOOT_READ_INT32_FROM_PROG_ADDR   [0x0C]
#define BOOT_LOAD_3_PARAMS   [0x0D]
#define BOOT_READ_CHECKSUM   [0x0E]
#define BOOT_RESET_CHECKSUM   [0x0F]
#define BOOT_AUX_SERVICES   [0x10]
#define BOOT_AUX_SERV_EN_DATA_ECHO   [0x1]
#define BOOT_AUX_SERV_2_APB_WRITES   [0x2]
#define BOOT_AUX_SERV_PLL_SERVICE   [0x3]
#define BOOT_AUX_SWITCH_AUX_INTERFACE   [0x4]
#define BOOT_AUX_RUN_AUX_PREDEF_CODE   [0x5]
#define BOOT_FLASH_SERVICES   [0x11]
#define BOOT_READ_VER_NUM   [0x12]
#define BOOT_READ_BLOCK_OF_DATA_MEM   [0x13]
#define BOOT_OUTPUTS_MICS_SAPLES_PTT_MODE   [0x14]
#define BOOT_PTT_MODE_EN_DDF0   0x1
#define BOOT_PTT_MODE_EN_DDF1   0x2
#define BOOT_PTT_MODE_EN_SAR_DDF0   0x4
#define BOOT_PTT_MODE_EN_SAR_DDF1   0x8
#define BOOT_PTT_MODE_UART_OUT_16_BIT   0x10
#define BOOT_PTT_MODE_UART_OUT_24_BIT   0x20
#define BOOT_PTT_MODE_TDM_OUT_16_BIT   0x40
#define BOOT_PTT_MODE_TDM_OUT_32_BIT   0x80


//------------------------
#define GAIN_OUTPUT_SCALE_DB_00  0x0100
#define GAIN_OUTPUT_SCALE_DB_01  0x011F
#define GAIN_OUTPUT_SCALE_DB_02  0x0142
#define GAIN_OUTPUT_SCALE_DB_03  0x016A
#define GAIN_OUTPUT_SCALE_DB_04  0x0196
#define GAIN_OUTPUT_SCALE_DB_05  0x01C7
#define GAIN_OUTPUT_SCALE_DB_06  0x01FF
#define GAIN_OUTPUT_SCALE_DB_07  0x023D
#define GAIN_OUTPUT_SCALE_DB_08  0x0283
#define GAIN_OUTPUT_SCALE_DB_09  0x02D2

#define GAIN_OUTPUT_SCALE_DB_10 0x032A
#define GAIN_OUTPUT_SCALE_DB_11 0x038C
#define GAIN_OUTPUT_SCALE_DB_12 0x03FB
#define GAIN_OUTPUT_SCALE_DB_13 0x0478
#define GAIN_OUTPUT_SCALE_DB_14 0x0503
#define GAIN_OUTPUT_SCALE_DB_15 0x05A0
#define GAIN_OUTPUT_SCALE_DB_16 0x064F
#define GAIN_OUTPUT_SCALE_DB_17 0x0714
#define GAIN_OUTPUT_SCALE_DB_18 0x07F1
#define GAIN_OUTPUT_SCALE_DB_19 0x08EA
#define GAIN_OUTPUT_SCALE_DB_20 0x0A00

#define GAIN_OUT_SCALE_DB_NONE	0xFFFF


/*#define SENSORY_INITIALIZED 			0x41 */
/*#define SENSORY_DUALTHRESHOLD 			0x45 */
/*#define SENSORY_WORDID					0x5B */

/* CPLD address & registers */                
#define CPLD_I2C_ADDR	0x33                               
#define CPLD_OVERWRITE_PROJECT_DEFAULTS				0x80       
#define CPLD_REG0									0x0                    
#define MANGO_PROJECT								0x5                  
#define MELON_PROJECT_D4P							0x2                
#define MELON_PROJECT_D4 							0x5                
#define KIWI_PROJECT 								0x4                  
#define D10L_KIWI_PROJECT 							0x1              
#define D10L_FPGA_PROJECT							0xF                
#define D10L_PNS_PROJECT							0x2                
#define ORANGE_PROJECT 								0x1                
#define UART_TO_FTDI 								0x40                 
#define CPLD_USE_EXTERNAL_MICS_EVB_L_DBMDX			0x20     
#define CPLD_USE_EXTERNAL_MICS_EVB_L				0x10         
#define CPLD_USE_INTERNAL_MICS						0x0            
#define CPLD_REG1									0x1                    
#define CPLD_DIFF_MCLK_CONFIG 						0x40           
#define CPLD_SAME_MCLK_CONFIG 						0x0            
#define CPLD_APR_MCLK_32K 							0x0              
#define CPLD_APR_MCLK_24M 							0x8              
#define CPLD_APR_MCLK_EXT 							0x10             
#define CPLD_APR_MCLK_NO_CLK 						0x18             
#define CPLD_APR_MCLK_HOST 							0x20             
#define CPLD_APR_MCLK_12M 							0x28             
#define CPLD_VT_MCLK_OR_UNI_MCLK_32K 				0x0          
#define CPLD_VT_MCLK_OR_UNI_MCLK_24M				0x1          
#define CPLD_VT_MCLK_OR_UNI_MCLK_EXT				0x2          
#define CPLD_VT_MCLK_OR_UNI_MCLK_NO_CLK				0x3        
#define CPLD_VT_MCLK_OR_UNI_MCLK_HOST				0x4          
#define CPLD_VT_MCLK_OR_UNI_MCLK_12M				0x5          
#define CPLD_REG2									0x2                    
#define CPLD_D2_TDM3_SRC_HOST_PB					0x0            
#define CPLD_D2_TDM3_SRC_MINI_STREAMER_PB			0x10       
#define CPLD_D2_TDM3_SRC_HOST_FULL_DPLX				0x20       
#define CPLD_D2_TDM3_SRC_MINI_STREAMER_FULL_DPLX	0x30   
#define CPLD_D2_TDM3_SRC_EXT_VIA_LED 0x40                
#define CPLD_D2_TDM3_SRC_GPIO_FLOAT 0x50                 
#define CPLD_D2_TDM3_SRC_DAC_CONNECTED_ONLY_DSP 0x60     
#define CPLD_D2_TDM3_SRC_CONNECTED_RPI_AND_DSP 0x70      
#define CPLD_TDM_HOST_CTL_DBMDX_MASTER 0x8               
#define CPLD_TDM_HOST_CTL_DBMDX_SLAVE 0x0                
#define CPLD_D4_D2_TDM0_SRC_HOST_PB 0x0                  
#define CPLD_D4_D2_TDM0_SRC_MINI_STREAMER_PB 0x0         
#define CPLD_D4_D2_TDM0_SRC_HOST_FULL_DPLX 0x2           
#define CPLD_D4_D2_TDM0_SRC_MINI_STREAMER_FULL_DPLX 0x2  
#define CPLD_D4_D2_TDM0_SRC_EXT_VIA_LED 0x4              
#define CPLD_D4_D2_TDM0_SRC_GPIO_FLOAT 0x5               
#define CPLD_D4_D2_TDM0_SRC_DAC_CONNECTED_ONLY_DSP 0x6   
#define CPLD_D4_D2_TDM0_SRC_DAC_CONNECTED_RPI_AND_DSP 0x7
#define CPLD_REG3 0x3                                    
#define CPLD_D5_UART_TX_SRC_D4 0x0                       
#define CPLD_D5_UART_TX_SRC_D2 0x1                       
#define CPLD_D5_I2C_D2_EN 0x2                            
#define CPLD_D5_I2C_D4_EN 0x4                            
#define CPLD_REG4 0x4                                    
#define CPLD_D2_D5_TDM_2_FLOAT 0x0                       
#define CPLD_D2_D5_TDM_2_TDM_0_SHORT 0x4                 
#define CPLD_D2_D5_TDM_2_DEFAULT 0xc                     
#define CPLD_D2_D5_D10_TDM_1_FLOAT 0x0                   
#define CPLD_D2_D5_D10_TDM_1_TDM_0_SHORT 0x1             
#define CPLD_D2_D5_TDM_1_SPI 0x2                         
#define CPLD_D2_D5_TDM_1_DEFAULT 0x3                     
#define CPLD_REG5 0x5                                    
#define CPLD_REG6 0x6                                    
#define CPLD_MANGO_VT_TDM_1_IS_LINK_MASTER 0x0           
#define CPLD_MANGO_APR_TDM_0_IS_LINK_MASTER 0x8          
#define CPLD_MELON_KIWI_TDM1_CLK_FSYNC_FLOAT 0x0         
#define CPLD_MELON_KIWI_TDM1_CLK_FSYNC_CONNECTED_TDM0 0x4
#define CPLD_MANGO_TDM2_CLK_FSYNC_FLOAT 0x0              
#define CPLD_MANGO_TDM2_CLK_FSYNC_CONNECTED_TDM3 0x4     
#define CPLD_MANGO_APR_TDM3_TX_TO_RPI 0x0                
#define CPLD_MANGO_VT_TDM0_TX_TO_RPI 0x2                 
#define CPLD_DBMDX_IS_DRIVING_CODEC 0x0                  
#define CPLD_HOST_IS_DRIVING_CODEC 0x1                   
#define CPLD_REG7 0x7                                    
#define CPLD_SPI_CONNECTED_TO_HOST 0x0                   
#define CPLD_SPI_CONNECTED_TO_FLASH 0x1                  
#define CPLD_SPI_CONNECTED_TO_FTDI 0x2                   
#define CPLD_SPI_FLOAT 0x3                               
#define CPLD_UART_CONNECTED_TO_HOST 0x4                  
#define CPLD_UART_FLOAT 0x0                              
#define CPLD_I2C_CONNECTED_TO_HOST 0x8                   
#define CPLD_I2C_FLOAT 0x0                               
#define CPLD_REG8 0x8                                    
#define CPLD_D4_D8_STRAP_0_HIGH 0x1                      
#define CPLD_D4_D8_STRAP_1_HIGH 0x2                      
#define CPLD_D4_D8_STRAP_2_HIGH 0x4                      
#define CPLD_D2_STRAP_0_HIGH 0x8                         
#define CPLD_D2_STRAP_1_HIGH 0x10                        
#define CPLD_D2_STRAP_2_HIGH 0x20                        
#define CPLD_D2_STRAP_3_HIGH 0x40                        
#define CPLD_REG9 0x9                                    
#define CPLD_VESPER_TH_84DB_OLD_72DB 0x0                 
#define CPLD_VESPER_TH_78DB_OLD_65DB 0x20                
#define CPLD_VESPER_TH_72DB 0x40                         
#define CPLD_VESPER_TH_65DB 0x30                         
#define CPLD_VESPER_DOUT_FLOAT 0x1F                      
#define CPLD_REG10 0xa                                   
#define CPLD_VESPER_MODE_FLOAT 0x1F                      
#define CPLD_REG12 0xC                                   
                                              
#define CPLD_DSP_GPIO_0 0x0                              
#define CPLD_DSP_GPIO_1 0x1                              
#define CPLD_DSP_GPIO_2 0x2                              
#define CPLD_DSP_GPIO_3 0x3                              
#define CPLD_DSP_GPIO_4 0x4                              
#define CPLD_DSP_GPIO_5 0x5                              
#define CPLD_DSP_GPIO_6 0x6                              
#define CPLD_DSP_GPIO_7 0x7                              
#define CPLD_DSP_GPIO_8 0x8                              
#define CPLD_DSP_GPIO_9 0x9                              
#define CPLD_DSP_GPIO_10 0xa                             
#define CPLD_DSP_GPIO_11 0xb                             
#define CPLD_DSP_GPIO_12 0xc                             
#define CPLD_DSP_GPIO_13 0xe                             
#define CPLD_DSP_GPIO_14 0xf                             
#define CPLD_DSP_GPIO_15 0x10                            
#define CPLD_DSP_GPIO_16 0x11                            
#define CPLD_DSP_GPIO_17 0x12                            
#define CPLD_DSP_GPIO_18 0x13                            
#define CPLD_DSP_GPIO_19 0x14                            
#define CPLD_DSP_GPIO_20 0x15                            
#define CPLD_DSP_GPIO_21 0x16                            
#define CPLD_DSP_GPIO_22 0x17                            
#define CPLD_DSP_GPIO_23 0x18                            
#define CPLD_DSP_GPIO_24 0x19                            

/* ASRP Registers for QED */

#define ASRP_USER_DEFINED_DELAY  0x107
#define ASRP_BLOCKID_LOW_PART 0x124
#define ASRP_BLOCKID_HIGH_PART 0x125
#define ASRP_ENABLE_OFFSET 0x126
#define ASRP_VALUE  0x127
#define ASRP_RECORDS_CHANNELS 0x128

#define ASRP_QED_Trigger_mode			0x140
#define ASRP_QED_Trigger_offset_mode 	0x141
#define ASRP_QED_Trigger_length 		0x142
#define ASRP_QED_OUT_detection 			0x143

#define           GAIN_OF_12_DB 					0x18
                              
/* command for AUDIO_BUFFER_SIZE (0x9) */
#define           BUFFER_SIZE_3_SEC 				0x1770
#define           BUFFER_SIZE_5_SEC 				0x2724
                              
/* command for LOAD_NEW_ACOUSTIC_MODEL (0xF) */
#define           LOAD_TRIGGER_ACOUSTIC_MODEL 					0x00
#define           LOAD_COMMAND_ACOUSTIC_MODEL 					0x01
#define           LOAD_COMMAND_ASRP_PARAM 					0x04
#define           AUDIO_BUFFER_READOUT_START_FROM_POINT_OF_SWITCHING_TO_BUFFERING 0x00
#define           AUDIO_BUFFER_READOUT_START_FROM_PHRASE_END_POINT 		0x01
#define           AUDIO_BUFFER_READOUT_START_FROM_PHRASE_START_POINT 		0x02
                              
/* command for HOST_INTERFACE_SUPPORT (0x29) */
#define           HOST_INTERFACE_I2C  0x02
#define           HOST_INTERFACE_UART 0x01
#define           HOST_INTERFACE_SPI  0x20

#define DBMD2             2
#define DBMD4 	        4

/* tdm addr */
#define            D4_TDM_0_RX_ADDR   0x80800000
#define            D4_TDM_1_RX_ADDR   0x80801000
                               
#define            D4_TDM_0_TX_ADDR   0x80804000
#define            D4_TDM_1_TX_ADDR   0x80805000
                               
#define            D2_TDM_0_RX_ADDR   0x80800000
#define            D2_TDM_1_RX_ADDR   0x80801000
#define            D2_TDM_2_RX_ADDR   0x80802000
#define            D2_TDM_3_RX_ADDR   0x80803000
                               
#define            D2_TDM_0_TX_ADDR   0x80804000
#define            D2_TDM_1_TX_ADDR   0x80805000
#define            D2_TDM_2_TX_ADDR   0x80806000
#define            D2_TDM_3_TX_ADDR   0x80807000
                               
#define            TDM3_TX_EN_BIT 0x8000
#define            TDM3_RX_EN_BIT 0x4000
                               
#define            TDM2_TX_EN_BIT 0x2000
#define            TDM2_RX_EN_BIT 0x1000
                               
                               
#define            TDM1_TX_EN_BIT 0x0800
#define            TDM1_RX_EN_BIT 0x0400
                               
#define            TDM0_TX_EN_BIT 0x0200
#define            TDM0_RX_EN_BIT 0x0100
                               
                               
#define            MODE_0_EN_BIT   0x0000
#define            MODE_1_EN_BIT   0x0004
#define            MODE_2_EN_BIT   0x0008


#define SED_GAIN				25
#define T3T4_GAIN 				25
#define DNN_STEP_SIZE			DNN_STEP_SIZE_GB_HW
#define THUD_EN_DIS				FE_THUD_ENABLE
#define THUD_REPORT_EN_DIS		FE_THUD_REPORTING_ENABLE

#define NUM_OF_CLASSES			PP_NUM_OF_CLASSES   
#define CLASSES_TO_REPORT 		PP_CLASSES_TO_REPORT
#define DNN_OUT_SMOOTH  		PP_DNN_OUT_SMOOTH

#endif
