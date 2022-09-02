#ifndef _DSPG_H_
#define _DSPG_H_

#include "Arduino.h"
#include "audio.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "hex/cyberon_model.h"
#include "hex/dspg_fw.h"
#include "hex/sed_model.h"
#include "dspgreg.h"
#include <SPI.h>
#include "RAK_SPI.h"

#include "hex/AsrpParams_Melon_Aec_1Mic_1Spk_v393x_T42_P16.h"

/*================================================================*/
#undef USE_HIBERNATE
#define USE_FW_READY
#undef CUSTOMER_CODE
#define CUSTOMER_BOARD
#define ASRP_RECORDING
#undef BUFFERING_MODE_MIPS_OPTIMIZED
#undef FW_DEBUG_DISABLE
#define HOST_BOOT

#define USE_HIGHER_GPIO_AT_3V3		  	1			// 0 - Use Higher GPIO at 1.8 V, 1 - Use Higher GPIO at 3.3 V
#define USE_HIGHER_GPIO					1			// 0 - Only Use GPIO from 0 to 18, 1 - Use GPIO from 0 to 18 with Higher GPIOs 19 to 30
#define USE_TDM1_ON_HIGHER_GPIO			1
#define TDM_CONNECT_TO_CODEC        	1     		// 1 = CODEC is connected to D10L, 0 = CODEC is connected to Host
#define TDM_CLK_SHARED              	0     		// 1 = CLK and FSYNC are shared between TDM0 and TDM1

#define TDM1_MASTER						1

#define NSBF_CH_DLY   0x15

/*================================================================*/

#define FILE_CHUNK 	0x8000
#define COMMAND_TABLE_LENGTH  15

#define MODEL_TYPE_VT		0
#define MODEL_TYPE_ASRP		1
#define MODEL_TYPE_COMMAND 	2
#define UART_SYNC_LEN 16


const uint8_t D10L_SCK = SCK;
const uint8_t D10L_MOSI = MOSI;
const uint8_t D10L_MISO = MISO;
const uint8_t D10L_CS = SS;
#define FW_READY_PIN    WB_IO3
#define FW_RESET_PIN    WB_IO4
#define FW_INT_PIN      WB_IO1

#define DSP_LOG_ENABLED     5
#define DBG_PRINT_CMDLOG	5


typedef enum {
	apr_core	= 2,	//CHIP_DBMD2
	vt_core 	= 10,	//CHIP_DBMD10L
} core_t;

typedef enum {
	VAL_SHORT		= 0,		//16 Bits
	VAL_LONG		= 1,		//32 Bits
} dbmdx_reg_value_size_t;

typedef enum  {
	TRIGGER,
    COMMANDS
}trigger_type;
typedef enum  {
	WWE_NONE = 0,
	WWE_SENSORY,
	WWE_AMAZON,
	WWE_GOOGLE,
	WWE_SAMSUNG,
	WWE_SENSING_SED,
	WWE_T3T4,
	WWE_DUAL,
	WWE_RETUNE,
	WWE_CYBERON
}wake_engine;

typedef enum {
	USE_CASE_IDLE = 0
	,USE_CASE_BARGEIN_1MIC = 1
	,USE_CASE_ENTER_DECT_MODE = 2
	,USE_CASE_ENTER_BUFF_MODE = 3
	,USE_CASE_ENTER_HIBERNATE_MODE = 4
	,USE_CASE_EXIT_HIBERNATE_MODE = 5
	,USE_CASE_PRODUCTION_TEST = 6
	,USE_CASE_VC_1MIC = 7	
	,USE_CASE_TRIGGERED = 100
	,USE_CASE_READ_REG = 101
	,USE_CASE_WRITE_REG = 102
}DSPG_MODE;

typedef struct st_command {
    uint16_t    reg;
    uint32_t    val;
	dbmdx_reg_value_size_t	   value_size;   
} COMMAND;



class DSPG
{
public:
    DSPG();
    virtual ~DSPG();
    int begin(void) ;
    void readCheckSum(char *checksum , core_t core);
    void loadFile(const char * file, unsigned long fileLen, core_t core, int skip_bytes);
    int config(void); 
    void run(void);
    void registerConfig(void);
    void useCaseConfig(COMMAND *command, int command_sequence_length);
    void checkStatus(void);
    void getFwVersion(void);
    void getChipID(void);
    void modelLoad(void);
    void spiRegisterWrite(uint16_t reg, uint8_t *value, int length); 
    void spiRegisterRead(uint16_t reg, uint8_t *data,int length); 
    void spiWrite(char *value, int length);  
    void spiRead(char *value, int length); 
    uint32_t readCheckSumOnly(core_t core);
    uint32_t calcCheckSum(const char * file, unsigned long fileLen, int skip_bytes);
    void loadModelFile(const char * file, unsigned long fileLen, core_t core);
    void LoadModel(const char * file, unsigned long fileLen, int model_type, int mem_loc, int vt_num, int load_file_type);
    void eventCallBack(void);
    void mode(DSPG_MODE m);
    bool event = false;

    int ioExpanderInit(void);
    int sdCheck(void);
    int micCheck(void);
    int ampCheck(void);
    int dspCheck(void);
    int getDspIntState(void);
    int getDspReadyState(void);
    void setDspResetLow(void);
    void setDspResetHigh(void);
    void setSpiCsLow(void);
    void setSpiCsHigh(void);
    void setMicDirection(uint8_t dir);
    uint16_t readIoState(void);
    void echoCommands(void);
    void detectedCallback(void(*function)(void));

    void eventCallBack(char *command,int *command_id);



private:
    uint8_t fw_ok;    
    void fwReady();
    void assertCS(void);
    void deassertCS(void);
    void i2sInit(void);
    
    unsigned int atoh__(const char *String);
    uint16_t dbmdx_read_register(int16_t reg);
    void dbmdx_write_register(int16_t reg, int16_t val);
    uint32_t dbmdx_read_register_long(core_t core, int16_t reg);
    void dbmdx_write_register_long(core_t core, int16_t reg, int32_t val);
};



extern DSPG DSPG_USER;
extern COMMAND D10L_config_table[COMMAND_TABLE_LENGTH];
extern COMMAND RT_CMD_D10L_evb_bargein_1mic_enter[];
extern int RT_CMD_D10L_evb_bargein_1mic_enter_length;
extern COMMAND RT_CMD_D10L_evb_bargein_1mic_exit[];
extern COMMAND RT_CMD_D10L_evb_vc_1mic_enter1[];
extern COMMAND RT_CMD_D10L_evb_vc_1mic_enter2[];
extern COMMAND RT_CMD_D10L_evb_vc_1mic_exit[];
extern COMMAND RT_CMD_D10L_evb_production_test_enter[];
extern COMMAND RT_CMD_D10L_evb_production_test_exit[];
extern char cyberon_group1_commands[1][30];
extern char cyberon_group2_commands[40][30];

#endif