#include "audio.h"



Audio::Audio() {

}
void Audio::begin(void) {
	init_flash();
}
void Audio::GetBoardId(char *array){

#ifdef _VARIANT_RAK4630_
    static char szUUID[17];
	int *p1 ,*p2;
	p1 = (int *)ID1;
	p2 = (int *)ID2;
	for(int i =0;i<17;i++)
		szUUID[i] = 0;
	sprintf(szUUID,"%4X%4X",*p1,*p2);
	memcpy(array,szUUID,16);
#elif defined (_VARIANT_RAK11300_)
    static char szUUID[64];
	int i;
	uint32_t ints = save_and_disable_interrupts();
	uint8_t n = getUniqueSerialNumber((uint8_t *)szUUID);

	for (i = 0; i < n / 2; i++)
		szUUID[i] = szUUID[2 * i];
	szUUID[i] = 0;
	restore_interrupts(ints);
    memcpy(array,szUUID,16);
#elif defined (_VARIANT_RAK11200_)
    static char szUUID[17];
	int i;
	uint8_t id[8];
	uint32_t nMacLow;
	uint32_t nMacHigh;
	
	for(int i=0;i<17;i++)
		szUUID[i] = 0;
	for(int i=0;i<8;i++)
		id[i] = 0;
	
	// Using ESP32 MAC (48 bytes only, so 2 bytes will be 0)
	nMacLow = (*(volatile uint32_t*)(BLK0_RDATA1_REG));
    nMacHigh = (*(volatile uint32_t*)(BLK0_RDATA2_REG));
	id[7] = 0;
	id[6] = 0;
	id[5] = (uint8_t)(nMacLow);
	id[4] = (uint8_t)(nMacLow >> 8);
	id[3] = (uint8_t)(nMacLow >> 16);
	id[2] = (uint8_t)(nMacLow >> 24);
	id[1] = (uint8_t)(nMacHigh);
	id[0] = (uint8_t)(nMacHigh >> 8);
	
	sprintf(szUUID,"%02X%02X%02X%02X%02X%02X%02X%02X",id[0],id[1],id[2],id[3],id[4],id[5],id[6],id[7]);
    memcpy(array,szUUID,16);  
#endif
    
}

void Audio::StoreLicense(uint32_t *lic){
	flash_write(CYBERON_FILEPATH, lic, LICEENSE_LENGTH*4);
}


uint32_t* Audio::GetLicenseAddr(void){
    uint32_t tmp[46];
	flash_read(CYBERON_FILEPATH, tmp, LICEENSE_LENGTH*4);
	memcpy(this->Cyberon_License,tmp,LICEENSE_LENGTH*4);
	return this->Cyberon_License;
}

void Audio::GetLicense(uint32_t *lic){
	flash_read(CYBERON_FILEPATH, lic, LICEENSE_LENGTH*4);
}


Audio Audio_Handle;