/**
 * @file flash-rp2040.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize, read and write parameters from/to internal flash memory
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifdef ARDUINO_ARCH_RP2040
#include "audio.h"
LittleFS_MBED *myFS;

FILE *license_file;


/**
 * @brief Initialize access to RP2040 internal file system
 * 
 */
void init_flash(void)
{
	// Initialize Internal File System
	myFS = new LittleFS_MBED();
	if (!myFS->init())
	{
		return;
	}
}


void flash_write(char const *filepath, uint32_t *lic, uint32_t length)
{
	license_file = fopen(filepath, "w");
	fwrite((uint8_t *)lic, 1, length, license_file);
	fclose(license_file);
}

void flash_read(char const *filepath, uint32_t *lic, uint32_t length)
{
	license_file = fopen(filepath, "r");
	fread((uint8_t *)lic, 1, length, license_file);
	fclose(license_file);
}

#endif