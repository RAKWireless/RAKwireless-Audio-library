/**
 * @file flash-esp32.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize, read and write parameters from/to internal flash memory
 * @version 0.1
 * @date 2021-01-10
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifdef ESP32
#include "audio.h"
#include <Preferences.h>

/** ESP32 preferences */
Preferences license_prefs;

/**
 * @brief Initialize access to ESP32 preferences
 *
 */
void init_flash(void)
{

}


void flash_write(char const *filepath, uint32_t *lic, uint32_t length)
{
    license_prefs.begin(filepath);
	license_prefs.putBytes("license", (uint8_t *)lic, length);
	license_prefs.end();
}

void flash_read(char const *filepath, uint32_t *lic, uint32_t length)
{
    license_prefs.begin(filepath);
	license_prefs.getBytes("license", (uint8_t *)lic, length);
	license_prefs.end();
}


#endif // ESP32