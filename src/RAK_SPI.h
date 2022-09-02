#ifndef RAK_SPI_H
#define RAK_SPI_H

#include <Arduino.h>

class RAK_SPI
{
public:
    void _openSPI(uint8_t slaveSelectPIN);
    void _closeSPI(uint8_t slaveSelectPIN);
    void begin();
    void end();
    void SPIselect(uint8_t slaveSelectPIN);
    void writeToSPI(uint8_t slaveSelectPIN, uint8_t headerLen, byte header[], uint16_t dataLen, byte data[]);
    void readFromSPI(uint8_t slaveSelectPIN, uint8_t headerLen, byte header[], uint16_t dataLen, byte data[]);
    void read(char *buff,int length);
    void write(char *buff,int length);
    void slow_write(char *buff,int length);
    void slow_read(char *buff,int length); 

private:

};

#endif