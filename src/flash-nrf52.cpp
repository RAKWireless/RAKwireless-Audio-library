/**
 * @file flash-nrf52.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize, read and write parameters from/to internal flash memory
 * @version 0.1
 * @date 2021-01-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifdef NRF52_SERIES

#include "audio.h"
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

File license_file(InternalFS);

/**
 * @brief Initialize access to nRF52 internal file system
 * 
 */
void init_flash(void)
{
	// Initialize Internal File System
	InternalFS.begin();
}


void flash_write(char const *filepath, uint32_t *lic, uint32_t length)
{
	InternalFS.remove(filepath);
	license_file.open(filepath, FILE_O_WRITE);
	license_file.write((uint8_t *)lic, length);
	license_file.flush();
	license_file.close();
}

void flash_read(char const *filepath, uint32_t *lic, uint32_t length)
{
	license_file.open(filepath, FILE_O_READ);
	license_file.read((uint8_t *)lic, length);
	license_file.close();
}

#endif