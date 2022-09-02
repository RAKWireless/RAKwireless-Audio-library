#ifndef AUDIO_H
#define AUDIO_H

#include "Arduino.h"
#include <stdint.h>
#include <string.h>
#include <PDM.h>
#include <I2S.h>
#include <SPI.h>
#include "tas2560.h"
#include "dspg.h"
#include "tpt29555.h"
#include "NAU85L40B.h"

#ifdef _VARIANT_RAK4630_
#include <nrf_nvic.h>
#define ID1 (0x10000060)
#define ID2 (0x10000064)

#define PDM_DATA_PIN  21
#define PDM_CLK_PIN   4
#define PDM_PWR_PIN   -1

#define BUFFER_SIZE   256
#define CYBERON_FILEPATH    "Cyberon_License"

#elif defined (_VARIANT_RAK11300_)
extern "C"
{
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/bootrom.h"
}
#include <LittleFS_Mbed_RP2040.h>

#define PDM_DATA_PIN  7
#define PDM_CLK_PIN   28
#define PDM_PWR_PIN   -1

#define BUFFER_SIZE   64
#define CYBERON_FILEPATH   MBED_LITTLEFS_FILE_PREFIX "/Cyberon_License.bin"
#elif defined (_VARIANT_RAK11200_)
#include <driver/i2s.h>
#include <Preferences.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_system.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "esp_now.h"

#define BUFFER_SIZE   256
#define DR_REG_BASE                       0x3ff5A000
#define BLK0_RDATA1_REG          (DR_REG_BASE + 0x004)
#define BLK0_RDATA2_REG          (DR_REG_BASE + 0x008)
#define CYBERON_FILEPATH    "Cyberon_License"   
#endif

// Flash
void init_flash(void);
void flash_write(char const *filepath, uint32_t *lic, uint32_t length);
void flash_read(char const *filepath, uint32_t *lic, uint32_t length);


#define LICEENSE_LENGTH   46


class Audio{

public:
  // constructors
  Audio();
  void begin(void);
  void GetBoardId(char *array);
  void StoreLicense(uint32_t *lic);
  uint32_t* GetLicenseAddr();
  void GetLicense(uint32_t *lic);  
  uint32_t Cyberon_License[LICEENSE_LENGTH];
private:

};

extern Audio Audio_Handle;
#endif