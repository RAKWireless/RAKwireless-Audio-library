#include "dspg.h"
#include "tpt29555.h"
#include "audio.h"

#if defined(_VARIANT_RAK11300_) // for RAK11300
  I2S RP2040_I2S(OUTPUT); 
#endif			

TPT29555   tpt29555_1(0x23);    //interrupt pin in this chip
TPT29555   tpt29555_2(0x25);

RAK_SPI  SPI_USER;

int cyberon_G1_count = 0;
int cyberon_G2_count = 0;

int fw_ready_configured = 0;
int g_wake_word_engine = WWE_DUAL;

uint8_t dsgp_init_flag = 0;

DSPG::DSPG()
{
	event = false;
}
DSPG::~DSPG()
{

}
void DSPG::i2sInit(void)
{
#if defined(_VARIANT_RAK11200_)
  I2S.setSampleBit(16);
  I2S.begin(2, 16000);
#elif defined(_VARIANT_RAK11300_) // for RAK11300
  RP2040_I2S.setBitsPerSample(16); 
 // start I2S at the sample rate with 16-bits per sample
  RP2040_I2S.begin(16000);
#else							  // for RAK4630
  I2S.begin(Stereo,16000,16);
  I2S.start();
 #endif
}
void DSPG::i2sEnd(void)
{
#if defined(_VARIANT_RAK11200_)  
  I2S.end();
#elif defined(_VARIANT_RAK11300_) // for RAK11300
  RP2040_I2S.end();
#else							  // for RAK4630  
  I2S.end();
 #endif
}
int DSPG::ioExpanderInit(void)
{

if (!tpt29555_1.begin())
  {
    Serial.println("Do not find IO Expander Chip1");
    return 1;
  }

  if (!tpt29555_2.begin())
  {
    Serial.println("Do not find IO Expander Chip2");
    return 1;    
  }
	
	tpt29555_1.pinMode(0, INPUT);	//SD check
	tpt29555_1.pinMode(1, INPUT);	//MIC check
	tpt29555_1.pinMode(2, INPUT);	//MIC CTR1
	tpt29555_1.pinMode(3, INPUT);	//MIC CTR2
	tpt29555_1.pinMode(4, INPUT);	//AMP check
	tpt29555_1.pinMode(5, INPUT);	//AMP CTR1
	tpt29555_1.pinMode(6, INPUT);	//AMP CTR2
	tpt29555_1.pinMode(7, INPUT);	//AMP CTR3
	tpt29555_1.pinMode(8, INPUT);	//DSP check
	tpt29555_1.pinMode(9, INPUT);	//DSP CTR1	DSP int 
	tpt29555_1.pinMode(10, INPUT);	//DSP CTR2	DSP ready
	tpt29555_1.pinMode(11, OUTPUT);	//DSP CTR3	DSP reset
	tpt29555_1.pinMode(12, OUTPUT);	//DSP CTR4	not use
	tpt29555_1.pinMode(13, OUTPUT);	//DSP CTR5	not use
	tpt29555_1.pinMode(14, OUTPUT);	//NOT USE
	tpt29555_1.pinMode(15, OUTPUT); //NOT USE

	tpt29555_2.pinMode(0, OUTPUT);	//CORE	SPI CS1		DSP CS
	tpt29555_2.pinMode(1, OUTPUT);	//CORE	SPI CS2		
	tpt29555_2.pinMode(2, OUTPUT);	//CORE	SPI CS3
	tpt29555_2.pinMode(3, OUTPUT);	//PDM	switch CTR		1 to dsp   0 to core
	tpt29555_2.pinMode(4, OUTPUT);	//not use
	tpt29555_2.pinMode(5, OUTPUT);	//not use
	tpt29555_2.pinMode(6, OUTPUT);	//not use
	tpt29555_2.pinMode(7, OUTPUT);	//not use
	tpt29555_2.pinMode(8, OUTPUT);	//
	tpt29555_2.pinMode(9, OUTPUT);	//
	tpt29555_2.pinMode(10, OUTPUT);	//
	tpt29555_2.pinMode(11, OUTPUT);	//
	tpt29555_2.pinMode(12, OUTPUT);	//
	tpt29555_2.pinMode(13, OUTPUT);	//
	tpt29555_2.pinMode(14, OUTPUT);	//
	tpt29555_2.pinMode(15, OUTPUT); //

	return 0;
}
int DSPG::sdCheck(void)
{
	return (tpt29555_1.digitalRead(0));	
}
uint16_t DSPG::readIoState(void)
{
	// uint8_t pot[16] = {0};
	uint8_t pot0 = tpt29555_1.readGPIO(0);
	uint8_t pot1 = tpt29555_1.readGPIO(1);
	delay(5);
	// Serial.printf("pot0:%02X  pot1:%02X\r\n",pot0,pot1);
	uint16_t state = pot1;
	state = (state<<8)|pot0;	
	return state;	
}
int DSPG::micCheck(void)
{	
	return (tpt29555_1.digitalRead(1));		//1 exist	 0 no exist
}
int DSPG::ampCheck(void)	
{
	return (tpt29555_1.digitalRead(4));		//1 exist	 0 no exist
}
int DSPG::dspCheck(void)
{
	return (tpt29555_1.digitalRead(8));		//1 exist	 0 no exist
}
int DSPG::getDspIntState(void)
{
	return (tpt29555_1.digitalRead(9));
}
int DSPG::getDspReadyState(void)
{
	return (tpt29555_1.digitalRead(10));
}
void DSPG::setDspResetLow(void)
{
	tpt29555_1.digitalWrite(11, 0);	//DSP CTR3	DSP reset
}
void DSPG::setDspResetHigh(void)
{
	tpt29555_1.digitalWrite(11, 1);	//DSP CTR3	DSP reset
}
void DSPG::setSpiCsLow(void) 
{
	tpt29555_2.digitalWrite(0, 0);	//DSP SPI CS
}
void DSPG::setSpiCsHigh(void)
{
	tpt29555_2.digitalWrite(0, 1);	//DSP CTR2	SPI CS
}
void DSPG::setSD_CS_High(void)
{
	tpt29555_2.digitalWrite(2, 1);
}
void DSPG::setSD_CS_Low(void)
{
	tpt29555_2.digitalWrite(2, 0);
}
void DSPG::setMicDirection(uint8_t dir)
{
	if(dir==0)
	{
		tpt29555_2.digitalWrite(3, 0);	//DSP CTR2	SPI CS
	}
	else
	{
		tpt29555_2.digitalWrite(3, 1);	//DSP CTR2	SPI CS
	}
	
}
int DSPG::begin(const char *lpdwModel,int modelSize) 
{
  //echo cancellation    
  i2sInit();
  delay(200);
  //power on
  Serial.println("IO expander init ...");

  if(ioExpanderInit())
  {
	Serial.println("RAK18003 module not detected, please check !\r\n");	
	return 1;
  }

 Serial.println("DSPG check");
  if(dspCheck()==0)
  {
	Serial.println("RAK18080 module not detected, please check RAK18080 !\r\n");	
	delay(200);
	return 1;
  }

  setMicDirection(1);

  //config the reset pin rx1 and reset
  setDspResetHigh();  
  delay(200);    
  setDspResetLow();
  delay(1000);
  setDspResetHigh();  

  fw_ok = 0;
  // begin SPI
  SPI_USER.begin();
  delay(100);

 Serial.println("DSPG config");
  if(config())
  {
	Serial.println("RAK18080 init failed, please check !\r\n");	
	return 1;
  }

  run(lpdwModel,modelSize);

  Serial.println("load model");
  mode(USE_CASE_BARGEIN_1MIC);
  
  event = false;

  // begin UART
//   Serial1.begin(921600);

  return 0;
}
void DSPG::end(void)
{
	SPI_USER.end();
	i2sEnd();
	delectDetectedCallback();
}
void DSPG::uartSync()
{
	uint32_t rc;
	char buf_sync[UART_SYNC_LEN] = {0};
	int i;
	char c[4] = {0};
	int size = 3;
	unsigned char buff_tmp[64] = {0};

	Serial.println("start boot sync");
	
	for (i = 0; i < size; i++) {
		rc = uartWrite(buf_sync, UART_SYNC_LEN);
		if (rc != UART_SYNC_LEN)
			Serial.println("sync buffer not sent correctly");

		Serial.println("sync buffer  sent correctly");
		uartRead(c, 4);
		Serial.println("sync read buffer");

        Serial.print("sync c:");
        Serial.print(c[0], HEX);    
        Serial.print(c[1], HEX);
        Serial.print(c[2], HEX);
        Serial.println(c[3], HEX);

		if(!strcmp("OK\n\r", c)) {
			Serial.println("UART Got Sync.");
			break;
		} else {
			Serial.println("Failed Sync UART");
		}
	}

	if(i == size) {
		Serial.println("Failed Sync UART");
	}
	Serial.println("boot sync successfully");
}

uint32_t DSPG::uartRegisterWrite(uint16_t reg, uint8_t *value, int length)  
{  
	char st[16];
	memset(st,0,16);
	sprintf(st,"%2xw%2x%2x", reg, value[0],value[1]);
	return uartWrite(st,7);
}

void DSPG::uartRegisterRead(uint16_t reg, uint8_t *data,int length) 
{
	char st[16];
	memset(st,0,16);
	sprintf(st,"%2xr", reg);
	uartWrite(st,3);
	fwReady();
	memset(st,0,16);	
	uartRead(st,2);
	data[0] = st[0];
	data[1] = st[1];	
}

uint32_t DSPG::uartWrite(char *value, int length)  
{  
	uint32_t i = Serial1.write(value,length); 
  	return i;
}

void DSPG::uartRead(char *value, int length)  
{  
	int timeout = 100;
	uint8_t i = 0;
	while (timeout--)
	{
		if (Serial1.available() > 0)
		{
			value[i++] = char(Serial1.read());
		}
	}
}


void DSPG::detectedCallback(void(*function)(void))
{
 //config interrupt
 pinMode(FW_INT_PIN, INPUT_PULLUP); 
 attachInterrupt(FW_INT_PIN, function,CHANGE);
}
void DSPG::delectDetectedCallback(void)
{
  detachInterrupt(FW_INT_PIN);
}
void DSPG::readCheckSum(char *checksum , core_t core)
{
	char write_buf[] = {0x5A, 0x0E};
	char c[7] = {0};
	uint32_t rc;

	spiWrite(write_buf,sizeof(write_buf));		
	delay(20);
	spiRead(c, 7);
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.println("read checksum");
		Serial.print("fw read checksum:");
		Serial.print(c[3], HEX);    
		Serial.print(c[4], HEX);
		Serial.print(c[5], HEX);
		Serial.println(c[6], HEX);
	}
	#endif
   	delay(20);

	if (strncmp(&c[3],checksum, 4))
    {        
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.print("checksum error:");
			Serial.print(checksum[0], HEX);    
			Serial.print(checksum[1], HEX);
			Serial.print(checksum[2], HEX);
			Serial.println(checksum[3], HEX);
		}
		#endif
		delay(20);
        fw_ok = 0;
    }    
	else
    {
        fw_ok = 1;
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.println("fw checksum pass");
		}
		#endif		
    }

}

uint32_t DSPG::readCheckSumOnly(core_t core)
{
	char write_buf[] = {0x5A, 0x0E};
	char c[7] = {0};
	uint32_t checksum;
	uint32_t rc;
	spiWrite(write_buf,sizeof(write_buf));	
	delay(50);
	spiRead(c, 7);

	checksum = (uint32_t)(c[3] & 0xff)
				| (uint32_t)(c[4] & 0xff) << 8
				| (uint32_t)(c[5] & 0xff) << 16
				| (uint32_t)(c[6] & 0xff) << 24;


	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.println("Read checksum of model");
		Serial.print("model checksum:");
    	Serial.println(checksum, HEX);    
	}
	#endif	

    return checksum;
}

void DSPG::loadFile(const char * file, unsigned long fileLen, core_t core, int skip_bytes)
{
	char *buffer;
	char *checksum = NULL;
	uint32_t rc;
	long err = 1;
	int i;
    char clr_crc_d4[]= {0x5a,0x0f};

	/*Allocate memory */
	buffer = (char *)file;
	
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.print("load filelen : ");
    	Serial.println(fileLen);
	}
	#endif	

	/* Upload File */
	if(skip_bytes > 0) {
		checksum = &buffer[fileLen - skip_bytes];
        spiWrite(clr_crc_d4,sizeof(clr_crc_d4));
	}

	for(i = 0 ; i < (fileLen - core); i += FILE_CHUNK) {
		int chunk ;
		chunk = (FILE_CHUNK < ((fileLen - skip_bytes) - i) ? FILE_CHUNK : (fileLen - skip_bytes) - i);
        spiWrite(&buffer[i],chunk);
		//spiWrite(clr_crc_d4,sizeof(clr_crc_d4));	
	}

	delay(10);
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.println("File Loading finished");
	}
	#endif    

	if(skip_bytes > 0){
		readCheckSum(checksum, core);
	}
}

int DSPG::config(void) 
{
	int fail_cnt = 0;
	uint32_t rsp = 0;
 	char send_buf_0[] = {0x00};

#ifdef CUSTOMER_BOARD
#if USE_HIGHER_GPIO
#if USE_HIGHER_GPIO_AT_3V3
	char preboot_request[] = {0x5A, 0x02, 0x05, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0xCD, 0xAB, 0x07, 0x00, 0x01, 0x00, 0x99, 0x9a, 0x00, 0x00};	//EVENT 25; FW READY 26; active high; use higher GPIOs for 68Pin at 3.3v
#else
	char preboot_request[] = {0x5A, 0x02, 0x05, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0xCD, 0xAB, 0x07, 0x00, 0x01, 0x00, 0x99, 0x9a, 0x02, 0x00};	//EVENT 25; FW READY 26; active high; use higher GPIOs for 68Pin at 1.8v
#endif
#else
	char preboot_request[] = {0x5A, 0x02, 0x04, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0xCD, 0xAB, 0x03, 0x00, 0x01, 0x00, 0x99, 0x9a};	//EVENT 25; FW READY 26; active high; don't use higher GPIOs
#endif
#else
	char preboot_request[] = {0x5A, 0x02, 0x05, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0xCD, 0xAB, 0x07, 0x00, 0x01, 0x00, 0x8e, 0x8a, 0x02, 0x00};	//EVENT 0x8e; FW READY 0x8a; active high
#endif

	// char preboot_request[] = {0x5A, 0x02, 0x05, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0xCD, 0xAB, 0x07, 0x00, 0x01, 0x00, 0x99, 0x9a, 0x02, 0x00};	//EVENT 25; FW READY 26; active high; use higher GPIOs for 68Pin at 1.8v

	char preboot_nnl1[] = {0x5a, 0x04, 0x90, 0x00, 0x00, 0x03, 0xA5, 0x52, 0x55, 0x55};
	char preboot_nnl2[] = {0x5a, 0x04, 0x94, 0x00, 0x00, 0x03, 0xA5, 0x52, 0x55, 0x15};
	char preboot_nnl3[] = {0x5a, 0x04, 0x20, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00}; 

	delay(100);	//delay(500);
    //write pre command
    spiWrite(send_buf_0,1);
    delayMicroseconds(150);
    spiWrite(preboot_request,sizeof(preboot_request));
    delay(5);    
    spiWrite(preboot_nnl1,sizeof(preboot_nnl1));
    delay(5);  
    spiWrite(preboot_nnl2,sizeof(preboot_nnl2));
    delay(5);
    spiWrite(preboot_nnl3,sizeof(preboot_nnl3));
    delayMicroseconds(150);
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.println("Start FW Loading...");
	}
	#endif
	
    /* Load Firmware */
    loadFile(dspg, sizeof(dspg), vt_core, 4);

	while(fw_ok == 0) {
        fail_cnt++;
        loadFile(dspg, sizeof(dspg), vt_core, 4);
        if (fw_ok == 1)
        {
			#if(DSP_LOG_ENABLED > 0)
			{
				Serial.println("FW Loading success");
			}
			#endif	            
            break;
        }
        if (fail_cnt == 5)
        {
		   #if(DSP_LOG_ENABLED > 0)
			{
				Serial.println("FW Loading fail");
			}
			#endif
            return 1;
            // while (1);
        }
    }
	
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.println("FW Loading success");
	}
	#endif
    return 0;
}

void DSPG::run(const char *lpdwModel,int modelSize)
{
    uint8_t rsp[2];
    uint16_t tmp;
	char buf_run[] = {0x5A, 0x0B};
	spiWrite(buf_run,2);
    delay(50);
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.println("FW run command");
	}
	#endif	
	
    spiRegisterRead((uint16_t)HOST_INTERFACE_SUPPORT,rsp,2);
    tmp = (uint16_t)(rsp[0] << 8) | rsp[1];    
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.print("HOST_INTERFACE_SUPPORT:");
    	Serial.println(tmp,HEX);
	}
	#endif   

	//#clear register 14
	do {
        spiRegisterRead((uint16_t)DETECT_AND_SYS_EVENTS,rsp,2);
        tmp = (uint16_t)(rsp[0] << 8) | rsp[1];
		if (PWRUP_COMP == tmp)
		{
            rsp[0] = 0x00;
            rsp[1] = 0x01;
			spiRegisterWrite(DETECT_AND_SYS_EVENTS, rsp,2);
			break;
		}
		delay(20);
	}while(1);   

	delay(30);

    getFwVersion();
    getChipID();
	
    //register config
    registerConfig();
    //check status
    checkStatus();
    //modelLoad    
	modelLoad(lpdwModel,modelSize); 
    //check status
    checkStatus();

	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.print("Ready to go!");
	}
	#endif
    
}

void DSPG::registerConfig(void) {

	COMMAND *command;
	int command_sequence_length;

	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.println("Init DSP Core Started!");
	}
	#endif	

	command = (COMMAND *)&D10L_config_table;
	command_sequence_length = sizeof(D10L_config_table)/sizeof(COMMAND);
	
  #if(DSP_LOG_ENABLED > 0)
	{
		Serial.printf("command_sequence_length:%d\r\n",command_sequence_length);
	}
	#endif	

    uint8_t rsp[4];
    int i;
    // for (i = 0; i < COMMAND_TABLE_LENGTH; i++)
	for (i = 0; i < command_sequence_length; i++)
    {
        if (D10L_config_table[i].value_size == VAL_SHORT)
        {
            rsp[0] = (uint8_t)((D10L_config_table[i].val & 0xFF00) >> 8);
            rsp[1] = (uint8_t)(D10L_config_table[i].val & 0x00FF);
            spiRegisterWrite(D10L_config_table[i].reg, rsp, 2);
        }
        else
        {
            rsp[0] = (uint8_t)((D10L_config_table[i].val & 0xFF000000) >> 24);
            rsp[1] = (uint8_t)((D10L_config_table[i].val & 0x00FF0000) >> 16);
            rsp[2] = (uint8_t)((D10L_config_table[i].val & 0x0000FF00) >> 8);
            rsp[3] = (uint8_t)(D10L_config_table[i].val & 0x000000FF);            
            spiRegisterWrite(D10L_config_table[i].reg, rsp, 4);
		
        }

		//check if fw ready gpio was configured
		if ((D10L_config_table[i].reg == EVENTS_INDICATION_GPIO) && ((D10L_config_table[i].val & 0xFF00) != 0x0000))
		{
			fw_ready_configured = 1;
			#if(DSP_LOG_ENABLED > 0)
			{
				Serial.println("fw_ready_configured");
			}
			#endif			
		}

		if ((D10L_config_table[i].reg == DSP_CLOCK_CONFIG) && (fw_ready_configured == 0))
		{
			delay(200);
		}

		if ((D10L_config_table[i].reg == OPERATION_MODE) && (fw_ready_configured == 0))
		{
			//Sometimes mode change takes more than 10ms, so, we need to delay extra 10ms here
			delay(10);
		}
    }
    delay(100);
}

void DSPG::useCaseConfig(COMMAND *command, int command_sequence_length){
    uint8_t rsp[4];
	uint8_t rspl[2];
    int i;
    for (i = 0; i < command_sequence_length; i++)
    {
        if (command[i].value_size == VAL_SHORT)
        {
            rspl[0] = (uint8_t)((command[i].val & 0xFF00) >> 8);
            rspl[1] = (uint8_t)(command[i].val & 0x00FF);
            spiRegisterWrite(command[i].reg, rspl, 2);
        }
        else
        {
            rsp[0] = (uint8_t)((command[i].val & 0xFF000000) >> 24);
            rsp[1] = (uint8_t)((command[i].val & 0x00FF0000) >> 16);
            rsp[2] = (uint8_t)((command[i].val & 0x0000FF00) >> 8);
            rsp[3] = (uint8_t)(command[i].val & 0x000000FF);            
            spiRegisterWrite(command[i].reg, rsp, 4);
        }

		//check if fw ready gpio was configured
		if ((command[i].reg == EVENTS_INDICATION_GPIO) && ((command[i].val & 0xFF00) != 0x0000))
		{
			fw_ready_configured = 1;
		}

		if ((command[i].reg == DSP_CLOCK_CONFIG) && (fw_ready_configured == 0))
		{
			delay(200);
		}

		if ((command[i].reg == OPERATION_MODE) && (fw_ready_configured == 0))
		{
			//Sometimes mode change takes more than 10ms, so, we need to delay extra 10ms here
			delay(10);
		}
    }
    delay(100);
}

void DSPG::checkStatus(void) {
    uint8_t rsp[2] = {0,0};
    uint32_t tmp;
    //READ_FW_ERROR_COUNTER is 0
    spiRegisterWrite((uint16_t)FW_STATUS_INDEX, rsp, 2);
    spiRegisterRead((uint16_t)IO_PORT_ADDRESS_LOW, rsp, 2);
    tmp = (uint16_t)((uint16_t)rsp[0] << 8) | rsp[1];
	
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.print("FW status:");
   		Serial.println(tmp,HEX);
	}
	#endif
    
}

void DSPG::getFwVersion(void) {
	char ver[16];
    uint8_t rsp[2];
    uint16_t tmp;
	memset(ver,0,16);
    spiRegisterRead((uint16_t)FW_VERSION_NUMBER,rsp,2);
    tmp = (uint16_t)(rsp[0] << 8) | rsp[1];
	sprintf(ver, "%X.",tmp);
    spiRegisterRead((uint16_t)FW_MINOR_NUMBER,rsp,2);
    tmp = (uint16_t)(rsp[0] << 8) | rsp[1];	
	sprintf(ver+strlen(ver), "%X_",tmp);	
    spiRegisterRead((uint16_t)FW_RC_NUMBER,rsp,2);
    tmp = (uint16_t)(rsp[0] << 8) | rsp[1];	

	#if(DSP_LOG_ENABLED > 0)
	{
		sprintf(ver+strlen(ver), "RC%X",tmp);	
		Serial.print("FW Version:");
		Serial.println(ver);
	}
	#endif	

}

void DSPG::getChipID(void) {
    uint8_t rsp[2];
    uint16_t tmp;
    spiRegisterRead((uint16_t)CHIP_ID_NUMBER,rsp,2);
    tmp = (uint16_t)(rsp[0] << 8) | rsp[1];
	
	#if(DSP_LOG_ENABLED > 0)
	{
    	Serial.print("CHIP ID:");
    	Serial.println(tmp,HEX);
	}
	#endif	
}

int DSPG::SetActiveCommandGroup(int nGroupActive)
{
	g_nActiveCommandsGroup = nGroupActive;
	if((g_nActiveCommandsGroup<=0) || (g_nActiveCommandsGroup>MAX_MODELS))
	{
		g_nActiveCommandsGroup = 1;
		return -1;
	}
	return g_nActiveCommandsGroup;
}

void DSPG::modelLoad(const char *lpdwModel,int modelSize) 
{
    uint8_t rsp[2];
    uint16_t tmp;
    spiRegisterRead((uint16_t)(VT_REG_INIT + VT1_REGS_OFFSET),rsp,2);
    tmp = (uint16_t)(rsp[0] << 8) | rsp[1];

	// verify for both models if they are already loaded
	if (tmp == 0)
	{
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.println("VT1 not initialized, assuming no model loaded. loading model");
		}
		#endif		

        rsp[0] = 0;
        rsp[1] = VT_ACTIVATE_GROUP_1_AND_2;
        spiRegisterWrite((uint16_t)(VT1_REGS_OFFSET | VT_REG_ACTIVATE_NUM_GROUPS), rsp, 2);
        rsp[0] = 0;
        rsp[1] = VT_ACTIVATE_MODEL_DSP;
        spiRegisterWrite((uint16_t)(VT1_REGS_OFFSET | VT_REG_ACTIVATE_MODEL_TYPE), rsp, 2);
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.printf("model size:%d\r\n",modelSize);
		}
		#endif			
		LoadModel(lpdwModel, modelSize, MODEL_TYPE_VT, MEM_TYPE_DTCM, LOAD_ENGINE_TYPE_VT1, FILE_TYPE_DTE_PRIM_MODEL);		
	
	}
	/*	disable VT2 model 
	//load sed model for VT2
    spiRegisterRead((uint16_t)(VT_REG_INIT + VT2_REGS_OFFSET),rsp,2);
    tmp = (uint16_t)(rsp[0] << 8) | rsp[1];
	if (tmp == 0)
	{
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.println("VT2 not initialized, assuming no model loaded. loading model");
		}
		#endif
		
		//config SED
		int vt_offset;
		vt_offset = VT_REG_INIT + VT2_REGS_OFFSET - 1;
		rsp[0] = 0;
        rsp[1] = THUD_EN_DIS;
   		spiRegisterWrite((uint16_t)(vt_offset + VT_FE_THUD_ENABLE), 			rsp, 2);
		rsp[0] = 0;
        rsp[1] = THUD_REPORT_EN_DIS;
        spiRegisterWrite((uint16_t)(vt_offset + VT_FE_THUD_REPORTING_ENABLE), 	rsp, 2);
		rsp[0] = 0;
        rsp[1] = DNN_STEP_SIZE_HW;		
        spiRegisterWrite((uint16_t)(vt_offset + VT_DNN_STEP_SIZE), 				rsp, 2);
		rsp[0] = 0;
        rsp[1] = NUM_OF_CLASSES;			
		spiRegisterWrite((uint16_t)(vt_offset + VT_PP_NUM_OF_CLASSES), 			rsp, 2);
		rsp[0] = 0;
        rsp[1] = CLASSES_TO_REPORT;	
		spiRegisterWrite((uint16_t)(vt_offset + VT_PP_CLASSES_TO_REPORT), 		rsp, 2);
		rsp[0] = 0;
        rsp[1] = 0x46;			
		spiRegisterWrite((uint16_t)(vt_offset + VT_FE_DNN_PATCH_SIZE), 			rsp, 2);


		LoadModel(sed, sizeof(sed), MODEL_TYPE_VT, MEM_TYPE_AHB, LOAD_ENGINE_TYPE_VT2, FILE_TYPE_DTE_PRIM_MODEL);
	}
	*/

}
uint32_t DSPG::calcCheckSum(const char * file, unsigned long fileLen, int skip_bytes)
{
	unsigned long currentfilePosition = 0;
	char *buffer;
	char header_buffer[10];
	int i;
	int j, length;
	uint32_t checksum = 0;
	uint32_t word_size = 0;
	uint32_t byte_size = 0;

	for(i = 0; i < 10; i++) 
	{
		memcpy(header_buffer, file+ currentfilePosition, 10);
		currentfilePosition += 10;
		
		if(header_buffer[0] == 0x5A && (header_buffer[1] != 0x01 && header_buffer[1] != 0x02)) {
			//printf("[%s]: header error.\n", __func__);
			break;
		}

		checksum += (uint32_t)(header_buffer[0] & 0xFF);
		checksum += (uint32_t)(header_buffer[1] & 0xFF);

		word_size = ((uint32_t)(header_buffer[5] & 0xFF) << 24)
				| ((uint32_t)(header_buffer[4] & 0xFF) << 16)
				| ((uint32_t)(header_buffer[3] & 0xFF) << 8)
				| ((uint32_t)(header_buffer[2] & 0xFF) << 0);
		checksum += word_size;

		byte_size = ((uint32_t)(header_buffer[9] & 0xFF) << 24)
				| ((uint32_t)(header_buffer[8] & 0xFF) << 16)
				| ((uint32_t)(header_buffer[7] & 0xFF) << 8)
				| ((uint32_t)(header_buffer[6] & 0xFF) << 0);
		checksum += byte_size;

		buffer = (char *)file + currentfilePosition;
		currentfilePosition += word_size * 2;

		length = word_size*2;
		for(j = 0 ; j < (length); j += 2) {
			byte_size = ((uint16_t)(buffer[j+1] & 0xFF)  << 8) 
					| ((uint16_t)(buffer[j] & 0xFF) << 0);
			checksum += byte_size;
		}
	} /* for(i = 0; i < 2; i++) */

	checksum += 104;

	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.print("calc checksum =");
		Serial.println(checksum,HEX);
	}
	#endif
	
	return checksum;
}

void DSPG::loadModelFile(const char * file, unsigned long fileLen, core_t core){
	unsigned long currentfilePosition = 0;
	char *buffer;
	char header_buffer[10];
	unsigned int num_of_words;
	int i;
	int j, length;
	int rc;

	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.print("loadModelFile fileLen =");
		Serial.println((unsigned int)fileLen);
	}
	#endif

	for(i = 0; i < 10; i++) 
	{
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.print("currentfilePosition =");
			Serial.println(currentfilePosition);
		}
		#endif
	
		/* fread(header_buffer, sizeof(header_buffer), 1, file); */
		memcpy(header_buffer, file + currentfilePosition, 10);
		currentfilePosition += 10;
				
		if(header_buffer[0] == 0x5A && (header_buffer[1] != 0x01 && header_buffer[1] != 0x02)) 
		{
			#if(DSP_LOG_ENABLED > 0)
			{
				// Serial.println("loadModelFile header error");
			}
			#endif			
			goto out;
		}
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.print("loadModelFile i =");
			Serial.println(i);
		}
		#endif		

		spiWrite(&header_buffer[0], sizeof(header_buffer));

		delay(20);

		num_of_words = ((uint32_t)(header_buffer[5] & 0xFF) << 24)
				| ((uint32_t)(header_buffer[4] & 0xFF) << 16)
				| ((uint32_t)(header_buffer[3] & 0xFF) << 8)
				| ((uint32_t)(header_buffer[2] & 0xFF) << 0);

		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.print("loadModelFile num_of_words =");
			Serial.println(num_of_words);
		}
		#endif		

		buffer = (char *)file + currentfilePosition;
		currentfilePosition += num_of_words * 2;

		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.print("num_of_words * 2 =");
			Serial.print(buffer[num_of_words * 2 -4],HEX);	
			Serial.print(buffer[num_of_words * 2 -3],HEX);
			Serial.print(buffer[num_of_words * 2 -2],HEX);
			Serial.println(buffer[num_of_words * 2 -1],HEX);
		}
		#endif		

		length = num_of_words*2;
		for(j = 0 ; j < (length); j += FILE_CHUNK) {
			int chunk;
			chunk = (FILE_CHUNK < ((length) - j) ? FILE_CHUNK : (length) - j);
			spiWrite(&buffer[j], chunk);
		}
		// if (i == 2)
		// {
		// 	header_buffer[0] == 0x5A;
		// 	header_buffer[1] == 0x8B;
		// 	spiWrite(&header_buffer[0], 2);
		// }
		
		delay(1);
	}
out:
	return;
}

void DSPG::LoadModel(const char * file, unsigned long fileLen, int model_type, int mem_loc, int vt_num, int load_file_type) {
    int blk_start;
    uint8_t rsp[2];
    uint16_t tmp;
	uint16_t lib_version, asrp_tun_version, asrp_errs;
	uint16_t offset = 0;
	uint16_t vt_init;
	uint32_t checksum_from_calculation;
	uint32_t checksum_from_dbmdx;

	if ((model_type != MODEL_TYPE_VT) && (model_type != MODEL_TYPE_COMMAND))
	{
		blk_start = BLK_START_NUM_0;
	}
	else
	{
		blk_start = BLK_START_NUM_1;
	}

    tmp = vt_num | OP_TYPE_LOAD_FILE | FILE_LOAD_INIT_OR_KILL_REL |blk_start | mem_loc | load_file_type;
    rsp[0] = (uint8_t)((tmp & 0xFF00) >> 8);
    rsp[1] = (uint8_t)(tmp & 0x00FF);
	spiRegisterWrite((uint16_t)(LOAD_BINARY_FILE), rsp, 2);

	delay(20);
	
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.println("Start uploading Model");
	}
	#endif	
	
	loadModelFile(file, fileLen, vt_core);

	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.println("Done uploading model.");
	}
	#endif	

	checksum_from_calculation = calcCheckSum(file, fileLen, 2);
	checksum_from_dbmdx = readCheckSumOnly(vt_core);

	if (checksum_from_calculation == checksum_from_dbmdx)
	{
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.print("checksum got =");
			Serial.println(checksum_from_dbmdx,HEX);
			Serial.print("checksum expected =");
			Serial.println(checksum_from_calculation,HEX);			
			// printf ("[%s]: checksum pass : got: = 0x%08x expected : 0x%08x\n",
			// 	__func__, checksum_from_dbmdx,checksum_from_calculation);
			Serial.println("checksum pass");
		}
		#endif

	}
	else
	{
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.print("checksum got =");
			Serial.println(checksum_from_dbmdx,HEX);
			Serial.print("checksum expected =");
			Serial.println(checksum_from_calculation,HEX);	
			// printf ("[%s]: checksum error : got: = 0x%08x expected : 0x%08x\n",
			// 	__func__, checksum_from_dbmdx,checksum_from_calculation);
			Serial.println("checksum error");
		}
		#endif

	}

	delay(20);
    //upload model begin 
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.println("model run");
	}
	#endif
	
	char buf_run[] = {0x5A, 0x0B};
	spiWrite(buf_run,2);
	delay(30);
    //upload model end 

	if (model_type == MODEL_TYPE_ASRP)
	{
		/* read ASRP lib version */
		// lib_version = dbmdx_host_read_register(core, ASRP_LIB_VER);

		memset(rsp,0,2);
		spiRegisterRead((uint16_t)ASRP_LIB_VER,rsp,2);
		lib_version = (uint16_t)(rsp[0] << 8) | rsp[1];
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.printf("ASRP LIB Version:%04X\r\n",lib_version);
		}
		#endif		

		if (lib_version == 0) 
		{
			#if(DSP_LOG_ENABLED > 0)
			{
				Serial.printf("[%s]: Asrp model was not successfully uploaded !, lib Version: 0x%04X\n", __func__, lib_version);
			}
			#endif
			
			return;
		}

		/* read ASRP tuning version */
		// asrp_tun_version = dbmdx_host_read_register(core, ASRP_TUNING_VER);
		
		memset(rsp,0,2);
		spiRegisterRead((uint16_t)ASRP_TUNING_VER,rsp,2);
		asrp_tun_version = (uint16_t)(rsp[0] << 8) | rsp[1];

		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.printf("ASRP TUNING Version:%04X\r\n",asrp_tun_version);
		}
		#endif
		
		if (asrp_tun_version == 0) 
		{
			#if(DSP_LOG_ENABLED > 0)
			{
				Serial.printf("[%s]: Asrp tunning version was not successfully gotten !, Tuning %s\n", __func__, asrp_tun_version);
			}
			#endif			
			return;
		}
		
		/* checking if loading asrp model has errors*/
		// asrp_errs = dbmdx_host_read_register(core, ASRP_NUM_OF_ERRS);

		memset(rsp,0,2);
		spiRegisterRead((uint16_t)ASRP_NUM_OF_ERRS,rsp,2);
		asrp_errs = (uint16_t)(rsp[0] << 8) | rsp[1];
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.printf("ASRP NUM OF ERRS:%04X\r\n",asrp_errs);
		}
		#endif		

		if (asrp_errs > 0) 
		{
			#if(DSP_LOG_ENABLED > 0)
			{
				Serial.printf("[%s]: ASRP Params loaded but have errors = %d !\n", __func__, asrp_errs);
			}
			#endif			
			return;
		}

		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.printf("[%s]: ASRP Loaded!, lib Version: 0x%04X, Tuning\r\n", __func__, lib_version);
		}
		#endif
		
	}
	else
	{
		if (vt_num == LOAD_ENGINE_TYPE_VT1)
		{
			offset = VT1_REGS_OFFSET;
		}
		else if (vt_num == LOAD_ENGINE_TYPE_VT2)
		{
			offset = VT2_REGS_OFFSET;
		}
		else if (vt_num == LOAD_ENGINE_TYPE_VT3)
		{
			offset = VT3_REGS_OFFSET;
		}
		
		/* verify loading acoustic model */
		spiRegisterRead((uint16_t)(VT_REG_INIT + offset),rsp,2);
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.printf("%X%X\r\n",rsp[0],rsp[1]);
		}
		#endif
		
		vt_init = (uint16_t)(rsp[0] << 8) | rsp[1];
		if (vt_init != 0x01) 
		{
			#if(DSP_LOG_ENABLED > 0)
			{
				Serial.println("Trigger Acoustic model was not successfully uploaded");
			}
			#endif			
		} else 
		{
			#if(DSP_LOG_ENABLED > 0)
			{
				Serial.println("Trigger Acoustic model was successfully uploaded");
			}
			#endif			
		}
	}	

}
void DSPG::spiRegisterWrite(uint16_t reg, uint8_t *value, int length)  
{  
	char st[16];
	memset(st,0,16);
	if (length == 2)
	{
		sprintf(st,"%03xw%02x%02x",reg,value[0],value[1]);
	}
	else{
		sprintf(st,"%03xW%02x%02x%02x%02x",reg,value[0],value[1],value[2],value[3]);
	}
	spiWrite(st,strlen(st));
	delay(10);
	//fwReady();
}

void DSPG::spiRegisterRead(uint16_t reg, uint8_t *data, int length) 
{
	char st[16];
	uint16_t tmp = 0;
	memset(st,0,16);
	sprintf(st,"%03xr", reg);
	#if(DSP_LOG_ENABLED >= 2)
	{
		Serial.print("Read Register:");	
		for (size_t i = 0; i < strlen(st); i++)
		{
			Serial.print(st[i]);	
		}
		Serial.println("");
	}
	#endif

	spiWrite(st,strlen(st));
	delay(10);
	//fwReady();
	memset(st,0,16);
	if (length == 2)
	{
		spiRead(st,5);
		#if(DSP_LOG_ENABLED >= 2)
		{
			Serial.print("Read Value:");	
		}
		#endif
		st[5] = 0;
		tmp = (uint16_t)atoi(&st[1]);
		#if(DSP_LOG_ENABLED >= 1)
		{
			Serial.println(tmp);
		}
		#endif
		data[0] = (tmp & 0xFF00) >> 8;
		data[1] = (tmp & 0x00FF);		/* code */
	}	
}

void DSPG::spiWrite(char *value, int length)  
{ 
	assertCS();
	delayMicroseconds(200);	//for RAK11300 this delay is necessary
	// if(dsgp_init_flag==1)
	// {
	// 	 SPI_USER.slow_write(value,length);
	// }
	// else
	// {
		 SPI_USER.write(value,length);
	// }   
	delayMicroseconds(200);
	deassertCS();
}

void DSPG::spiRead(char *value, int length)  
{  
	assertCS();
	delayMicroseconds(200);	
	// if(dsgp_init_flag==1)
	// {
	// 	SPI_USER.slow_read(value,length);
	// }
	// else
	// {
		SPI_USER.read(value,length);
	// }
	delayMicroseconds(200);
	deassertCS();
}

void DSPG::fwReady()
{
	while(digitalRead(FW_READY_PIN) == 1)
	{
		delay(1);
  		digitalWrite(LED_GREEN, HIGH);   
	}
  	digitalWrite(LED_GREEN, LOW);   	
}

void DSPG::assertCS(void)
{
	setSpiCsLow();
}

void DSPG::deassertCS(void)
{
  setSpiCsHigh();
}

void DSPG::echoCommands(void)
{
	int i;
	Serial.println("********Trigger Words***********");
	int n = sizeof(cyberon_trigger_commands)/sizeof(cyberon_trigger_commands[0]);	
	for(i=0;i<n;i++)
	{
	 Serial.println(cyberon_trigger_commands[i]);
	}	

	Serial.println("********Command Words*********");	

	switch(g_nActiveCommandsGroup)
	{
		case USE_MODEL_GROUP1:
			n = sizeof(g_cyberon_group1_commands)/sizeof(g_cyberon_group1_commands[0]);	
			for(i=0;i<n;i++)
			{
				Serial.printf("Command[%d]:\t%s\r\n",i,g_cyberon_group1_commands[i]);
			}
			break;
		case USE_MODEL_GROUP2:
			n = sizeof(g_cyberon_group2_commands)/sizeof(g_cyberon_group2_commands[0]);	
			for(i=0;i<n;i++)
			{
				Serial.printf("Command[%d]:\t%s\r\n",i,g_cyberon_group2_commands[i]);
			}
			break;
		case USE_MODEL_GROUP3:
			n = sizeof(g_cyberon_group3_commands)/sizeof(g_cyberon_group3_commands[0]);	
			for(i=0;i<n;i++)
			{			
				Serial.printf("Command[%d]:\t%s\r\n",i,g_cyberon_group3_commands[i]);
			}
			break;
		case USE_MODEL_GROUP4:
			n = sizeof(g_cyberon_group4_commands)/sizeof(g_cyberon_group4_commands[0]);	
			for(i=0;i<n;i++)
			{			
				Serial.printf("Command[%d]:\t%s\r\n",i,g_cyberon_group4_commands[i]);
			}
			break;
		default:
			n = sizeof(g_cyberon_group1_commands)/sizeof(g_cyberon_group1_commands[0]);	
			for(i=0;i<n;i++)
			{			
				Serial.printf("Command[%d]:\t%s\r\n",i,g_cyberon_group1_commands[i]);
			}
		break;
	}	
}
void DSPG::echoCommands(int nGroupChoose)
{
	int i;
	// char (*cmd)[50] ;	
	// char **cmd;
	Serial.println("********Trigger Words***********");
	
	int n = sizeof(cyberon_trigger_commands)/sizeof(cyberon_trigger_commands[0]);	
	for(i=0;i<n;i++)
	{
	 Serial.println(cyberon_trigger_commands[i]);
	}	

	Serial.println("********Command Words*********");	
	// cmd = getCommand(nGroupChoose,cmd);
	// cmd = getCommand(nGroupChoose);
	// n = sizeof(g_cyberon_group1_commands)/sizeof(g_cyberon_group1_commands[0]);	
	// cmd = g_cyberon_group1_commands;
	// for(i=0;i<15;i++)
	// {
	// 	Serial.printf("cmd[%d]:\t\t%s\r\n",i,cmd[i]);
	// }

	switch(nGroupChoose)
	{
		case USE_MODEL_GROUP1:	
			n = sizeof(g_cyberon_group1_commands)/sizeof(g_cyberon_group1_commands[0]);			
			for(i=0;i<n;i++)
			{
				Serial.printf("Command[%d]:\t%s\r\n",i,g_cyberon_group1_commands[i]);
			}
			break;
		case USE_MODEL_GROUP2:
			n = sizeof(g_cyberon_group2_commands)/sizeof(g_cyberon_group2_commands[0]);	
			for(i=0;i<n;i++)
			{
				Serial.printf("Command[%d]:\t%s\r\n",i,g_cyberon_group2_commands[i]);
			}
			break;
		case USE_MODEL_GROUP3:
			n = sizeof(g_cyberon_group3_commands)/sizeof(g_cyberon_group3_commands[0]);	
			for(i=0;i<n;i++)
			{			
				Serial.printf("Command[%d]:\t%s\r\n",i,g_cyberon_group3_commands[i]);
			}
			break;
		case USE_MODEL_GROUP4:
			n = sizeof(g_cyberon_group4_commands)/sizeof(g_cyberon_group4_commands[0]);	
			for(i=0;i<n;i++)
			{			
				Serial.printf("Command[%d]:\t%s\r\n",i,g_cyberon_group4_commands[i]);
			}
			break;
		default:
			n = sizeof(g_cyberon_group1_commands)/sizeof(g_cyberon_group1_commands[0]);	
			for(i=0;i<n;i++)
			{			
				Serial.printf("Command[%d]:\t%s\r\n",i,g_cyberon_group1_commands[i]);
			}
		break;
	}	
}

void DSPG::eventCallBack(char *command,int *command_id)
{
	uint16_t val;
	uint16_t mics = 0;
	int VT_offset;
	char rsp_str[100]; 
	int word_id = 0;

	//clear interrupt
	uint16_t interrupt_events = dbmdx_read_register((uint16_t)(DETECT_AND_SYS_EVENTS));
	delay(10);
	dbmdx_write_register(DETECT_AND_SYS_EVENTS,interrupt_events);

	#if(DSP_LOG_ENABLED >= 1)
	{
		Serial.println("===============================");	
		Serial.printf("event value:%d\r\n",interrupt_events);
	}
	#endif	
	delay(20);
	// if (event == VT1_DET) {VT_offset = VT1_REGS_OFFSET;}	
	// else if (event == VT2_DET) {VT_offset = VT2_REGS_OFFSET;}	
	VT_offset = VT1_REGS_OFFSET;		
	//get word ID
	word_id = dbmdx_read_register((uint16_t)(VT_offset | VT_REG_WORD_ID));   
	#if(DSP_LOG_ENABLED >= 1)
	{		
		Serial.print("Word ID:");
		Serial.println(word_id);	
	}
	#endif
	// delay(10);
	// uint16_t len = dbmdx_read_register((uint16_t)(VT_offset | VT_REG_PHRASE_LENGTH));	//this value is no used
    // Serial.print("Word ID Length:");
 	// Serial.println(len);	
	memset(rsp_str,0,100);
	// if(interrupt_events!=0)	
	if(interrupt_events == 2)	
	{
		*command_id = word_id;

		if (word_id < 2000)
		{
			cyberon_G1_count++;
			word_id -= 1001;
			
			// sprintf(rsp_str, "Trigger ! %s, trigger counts = %d",cyberon_trigger_commands[word_id],cyberon_G1_count);
			sprintf(rsp_str,"%s",cyberon_trigger_commands[word_id]);
			// #if(DSP_LOG_ENABLED >= 1)
			// {
			// 	Serial.printf("Trigger ! - word_id %d\r\n", word_id);			
			// 	Serial.println(rsp_str);	
			// 	Serial.println(cyberon_trigger_commands[word_id]);
			// }
			// #endif
			memcpy(command,cyberon_trigger_commands[word_id],sizeof(cyberon_trigger_commands[word_id]));
		}
		else 
		{
			cyberon_G2_count++;
			word_id -= 2001;	

			switch(g_nActiveCommandsGroup)
			{
				case USE_MODEL_GROUP1:	
					// sprintf(rsp_str, "Detecting ! %s, trigger counts = %d",g_cyberon_group1_commands[word_id],cyberon_G2_count);
					sprintf(rsp_str, "%s",g_cyberon_group1_commands[word_id],cyberon_G2_count);
					memcpy(command,g_cyberon_group1_commands[word_id],sizeof(g_cyberon_group1_commands[word_id]));	
					break;
				case USE_MODEL_GROUP2:
					// sprintf(rsp_str, "Detecting ! %s, trigger counts = %d",g_cyberon_group2_commands[word_id],cyberon_G2_count);
					sprintf(rsp_str, "%s",g_cyberon_group2_commands[word_id],cyberon_G2_count);
					memcpy(command,g_cyberon_group2_commands[word_id],sizeof(g_cyberon_group2_commands[word_id]));	
					break;
				case USE_MODEL_GROUP3:		
					// sprintf(rsp_str, "Detecting ! %s, trigger counts = %d",g_cyberon_group3_commands[word_id],g_cyberon_group3_commands);
					sprintf(rsp_str, "%s",g_cyberon_group3_commands[word_id],g_cyberon_group3_commands);
					memcpy(command,g_cyberon_group3_commands[word_id],sizeof(g_cyberon_group3_commands[word_id]));	
					break;
				case USE_MODEL_GROUP4:			
					// sprintf(rsp_str, "Detecting ! %s, trigger counts = %d",g_cyberon_group4_commands[word_id],g_cyberon_group4_commands);
					sprintf(rsp_str, "%s",g_cyberon_group4_commands[word_id],g_cyberon_group4_commands);
					memcpy(command,g_cyberon_group4_commands[word_id],sizeof(g_cyberon_group4_commands[word_id]));	
					break;
				default:	
				break;
			}						
		}
	}

	#if(DSP_LOG_ENABLED >= 1)
	{
		Serial.println("===============================");		
	}   
	#endif
	event = false;
	//mode(USE_CASE_ENTER_DECT_MODE);
}
void DSPG::mode(DSPG_MODE m)
{
	switch (m)
	{
	case USE_CASE_ENTER_DECT_MODE:
		break;
	case USE_CASE_TRIGGERED:
		break;
	case USE_CASE_BARGEIN_1MIC:
		#if(DSP_LOG_ENABLED > 0)
		{
			Serial.println();
			Serial.println("LoadModel AsrpParams_Melon_Aec_1Mic_1Spk_v393x_T42_P16");
		}
		#endif	

		LoadModel(AsrpParams_Melon_Aec_1Mic_1Spk_v393x_T42_P16, sizeof(AsrpParams_Melon_Aec_1Mic_1Spk_v393x_T42_P16), 
				MODEL_TYPE_ASRP, MEM_TYPE_AHB, LOAD_ENGINE_TYPE_ASRP, FILE_TYPE_ASRP_PARAMS_PRIM_INIT);
		useCaseConfig(RT_CMD_D10L_evb_bargein_1mic_enter,RT_CMD_D10L_evb_bargein_1mic_enter_length);
		break;
	default:
		break;
	}
	dsgp_init_flag = 1;
	#if(DSP_LOG_ENABLED > 0)
	{
		Serial.print("Work mode: ");
		Serial.println(m);
	}
	#endif	
	
}

/***********************************************************************
* FUNCTION NAME: atoh__(const char *String)
*
* DESCRIPTION:
*	Convert Ascci to Hex.
*
* PARAMETERS:
*	const char *String
*
* RETURNS:
*	integer value
***********************************************************************/
/*static */
unsigned int DSPG::atoh__(const char *String)
{
    int Value = 0, Digit;
    char c;

    while ((c = *String++) != '\0')
    {
        if (c >= '0' && c <= '9')
            Digit = (uint) (c - '0');
        else if (c >= 'a' && c <= 'f')
            Digit = (uint) (c - 'a') + 10;
        else if (c >= 'A' && c <= 'F')
            Digit = (uint) (c - 'A') + 10;
        else
            break;

        Value = (Value << 4) + Digit;
    }

    return Value;
}
uint16_t DSPG::dbmdx_read_register(int16_t reg)
{
	uint16_t val = 0;
	int ret;
	char Write_Read_Buff[10] = {0};
	char RegSet[18] = {0};
#if(DBG_PRINT_CMDLOG>=1) 	
	char reg_str[8];
	char val_str[8];
#endif

#if defined(_VARIANT_RAK11200_)
	SPI_USER.end();
	// begin SPI
  	SPI_USER.begin();
#endif

#if(DBG_PRINT_CMDLOG>=1) 
	sprintf(reg_str, "%03x", reg);
#endif

	sprintf(RegSet, "%03xr", reg);
	spiWrite(RegSet, strlen(RegSet));
	// #if(DBG_PRINT_CMDLOG>=1) 
	//  {
	// 	// Serial.printf("[%s]: device_write error.\n", __func__);		
	// 	Serial.printf("%s: \n", );		
	// }
	// #endif

#ifdef USE_FW_READY
	if (fw_ready_configured == 1)
	{
		delay(1);
		do{
			ret = getDspReadyState();
		}while ((ret < 0) || (ret == LOW)); 
	}
	else
#endif
	delay(10);

	spiRead(Write_Read_Buff, 5);	

	Write_Read_Buff[5] = 0;
	val = atoh__((char*)&Write_Read_Buff[1]);
	if(val < 0) {
		Serial.printf("[%s]: device_read error.\n", __func__);
		return 0;
	}


#if(DBG_PRINT_CMDLOG>=1) 
	sprintf(val_str, "%04x", (val)&0xffff);
	Serial.printf("DBMD Reading reg: 0x%s  ; value: 0x%s\n",reg_str, val_str);
#endif
	
	return val;
}

void DSPG::dbmdx_write_register(int16_t reg, int16_t val)
{
	int ret;
	char str[16];
#if(DBG_PRINT_CMDLOG>=1) 
	char reg_str[8];
	char val_str[8];
#endif

#if defined(_VARIANT_RAK11200_)
	SPI_USER.end();
	// begin SPI
  	SPI_USER.begin();
#endif

	sprintf(str, "%03xw%04x", reg, (val)&0xffff);
#if(DBG_PRINT_CMDLOG>=1) 
	sprintf(reg_str, "%03x", (reg)&0xfff);
	sprintf(val_str, "%04x", (val)&0xffff);
	Serial.printf("DBMD Writing reg: 0x%s  ; value: 0x%s\n",reg_str, val_str);
#endif
	spiWrite(str, strlen(str));
	
#ifdef USE_FW_READY
	if (fw_ready_configured == 1)
	{
		delay(1);
		do{
			ret = getDspReadyState();
		}while ((ret < 0) || (ret == LOW)); 
	}
	else
#endif
	delay(10);
}

uint32_t DSPG::dbmdx_read_register_long(core_t core, int16_t reg)
{
	uint32_t val = 0;
	int ret;
	char Write_Read_Buff[10] = {0};
	char RegSet[18] = {0};
#if(DBG_PRINT_CMDLOG==1) 
	char reg_str[8];
	char val_str[8];
#endif

#if(DBG_PRINT_CMDLOG==1) 
	sprintf(reg_str, "%03x", reg);
#endif

	sprintf(RegSet, "%03xR", reg);
	spiWrite(RegSet, strlen(RegSet));
	
#ifdef USE_FW_READY
	if (fw_ready_configured == 1)
	{
		delay(1);
		do{
			ret = getDspReadyState();
		}while ((ret < 0) || (ret == LOW)); 
	}
	else
#endif
	delay(10);

	spiRead(Write_Read_Buff, 9);
	
	Write_Read_Buff[9] = 0;
	val = atoh__((char*)&Write_Read_Buff[1]);
	if(val < 0) {
		Serial.printf("[%s]: device_read error.\n", __func__);
		return 0;
	}

#if(DBG_PRINT_CMDLOG==1) 
	sprintf(val_str, "%08x", (val)&0xffffffff);
	Serial.printf("DBMD Reading reg: 0x%s  ; value: 0x%s\n",reg_str, val_str);
#endif	
	return val;
}


void DSPG::dbmdx_write_register_long(core_t core, int16_t reg, int32_t val)
{
	int ret;
	char str[20];
#if(DBG_PRINT_CMDLOG==1) 
	char reg_str[10];
	char val_str[10];
#endif

	sprintf(str, "%03xW%08lx", reg, (val)&0xffffffff);
#if(DBG_PRINT_CMDLOG==1) 
	sprintf(reg_str, "%03x", (reg)&0xfff);
	sprintf(val_str, "%08x", (val)&0xffffffff);
	printf("DBMD Writing reg: 0x%s  ; value: 0x%s\n", reg_str, val_str);
#endif
	spiWrite(str, strlen(str));
	
#ifdef USE_FW_READY
	if (fw_ready_configured == 1)
	{
		delay(1);
		do{
			ret = getDspReadyState();
		}while ((ret < 0) || (ret == LOW)); 
	}
	else
#endif
	delay(10);
}




DSPG DSPG_USER;