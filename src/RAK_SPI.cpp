#include <Arduino.h>
#include <SPI.h>
#include "RAK_SPI.h"

const uint8_t RAK_SCK = SCK;
const uint8_t RAK_MOSI = MOSI;
const uint8_t RAK_MISO = MISO;
const uint8_t RAK_CS = SS;

#if defined(_VARIANT_RAK4630_)
#define BOARD "RAK4631"
SPIClass RAKSPI(NRF_SPIM3, RAK_MISO, RAK_SCK, RAK_MOSI);
#elif defined(_VARIANT_RAK11300_)
#define BOARD "RAK11300 "
MbedSPI RAKSPI(RAK_MISO, RAK_MOSI, RAK_SCK);
#else
#define BOARD "RAK11200 "
SPIClass RAKSPI;
#endif

#define FREQUENCY_500KHZ    500000     // 500KHz
#define FREQUENCY_1MHZ    1000000     // 1MHz
#define FREQUENCY_2MHZ    2000000     // 2MHz
#define FREQUENCY_4MHZ    4000000     // 4MHz
#define FREQUENCY_8MHZ    8000000     // 8MHz
#define FREQUENCY_20MHZ   20000000    // 20MHz

const SPISettings spiSettings = SPISettings(FREQUENCY_4MHZ, MSBFIRST, SPI_MODE0);
const SPISettings spiSettings1 = SPISettings(FREQUENCY_1MHZ, MSBFIRST, SPI_MODE0);

void RAK_SPI::_openSPI(uint8_t slaveSelectPIN)
{
  RAKSPI.beginTransaction(spiSettings);
  // take the chip select low to select the device
  digitalWrite(slaveSelectPIN, LOW);
}

void RAK_SPI::_closeSPI(uint8_t slaveSelectPIN)
{
  // take the chip select high to de-select, finish config write
  digitalWrite(slaveSelectPIN, HIGH);
  RAKSPI.endTransaction();
}

void RAK_SPI::begin()
{
#if defined(_VARIANT_RAK4630_)
  RAKSPI.begin();
#elif defined(_VARIANT_RAK11300_)
  RAKSPI.begin();
#else
  RAKSPI.begin(RAK_SCK, RAK_MISO, RAK_MOSI);
#endif
}

void RAK_SPI::end()
{
  RAKSPI.end();
}

void RAK_SPI::SPIselect(uint8_t slaveSelectPIN)
{
  pinMode(slaveSelectPIN, OUTPUT);
  digitalWrite(slaveSelectPIN, HIGH);
}

void RAK_SPI::writeToSPI(uint8_t slaveSelectPIN, uint8_t headerLen, byte header[], uint16_t dataLen, byte data[])
{
  _openSPI(slaveSelectPIN);
  for (auto i = 0; i < headerLen; i++)
  {
    RAKSPI.transfer(header[i]); // send header
  }
  for (auto i = 0; i < dataLen; i++)
  {
    RAKSPI.transfer(data[i]); // write values
  }
  delayMicroseconds(5);
  _closeSPI(slaveSelectPIN);
}

void RAK_SPI::readFromSPI(uint8_t slaveSelectPIN, uint8_t headerLen, byte header[], uint16_t dataLen, byte data[])
{
  _openSPI(slaveSelectPIN);
  for (auto i = 0; i < headerLen; i++)
  {
    RAKSPI.transfer(header[i]); // send header
  }
  for (auto i = 0; i < dataLen; i++)
  {
    data[i] = RAKSPI.transfer(0x00); // read values
  }
  delayMicroseconds(5);
  _closeSPI(slaveSelectPIN);
}

void RAK_SPI::write(char *buff,int length)
{
  RAKSPI.beginTransaction(spiSettings);
	for (size_t i = 0; i < length; i++)
	{
		RAKSPI.transfer((uint8_t)buff[i]);     
	}
	RAKSPI.endTransaction();
}
void RAK_SPI::read(char *buff,int length) 
{
  RAKSPI.beginTransaction(spiSettings);
	for (size_t i = 0; i < length; i++)
	{    
		buff[i] = RAKSPI.transfer((uint8_t)0);
	}
	RAKSPI.endTransaction();
}

void RAK_SPI::slow_write(char *buff,int length) 
{
  RAKSPI.beginTransaction(spiSettings1);
	for (size_t i = 0; i < length; i++)
	{
		RAKSPI.transfer((uint8_t)buff[i]);     
	}
	RAKSPI.endTransaction();
}
void RAK_SPI::slow_read(char *buff,int length) 
{
  RAKSPI.beginTransaction(spiSettings1);
	for (size_t i = 0; i < length; i++)
	{    
		buff[i] = (char)RAKSPI.transfer((uint8_t)0);
	}
	RAKSPI.endTransaction();
}