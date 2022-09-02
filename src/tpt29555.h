#ifndef _TPT29555_H_
#define _TPT29555_H_

#include "Arduino.h"
#include <Wire.h>


#define IO_EXPANDER_TPT29555_ADDRESS 0x23
//#define IO_EXPANDER_TPT29555_ADDRESS_2 0x25

#define TPT_OUTPUT   0x00
#define TPT_INTPUT   0x01

#define TPT29555_INPUT_PORT0_REGISTER 0x00 // this register is an input-only port
#define TPT29555_INPUT_PORT1_REGISTER 0x01
#define TPT29555_OUTPUT_PORT0_REGISTER 0x02
#define TPT29555_OUTPUT_PORT1_REGISTER 0x03
#define TPT29555_POLARITY_INVERSION_PORT0_REGISTER 0x04
#define TPT29555_POLARITY_INVERSION_PORT1_REGISTER 0x05
#define TPT29555_CONFIGURATION_PORT0_REGISTER 0x06
#define TPT29555_CONFIGURATION_PORT1_REGISTER 0x07

class TPT29555
{
public:
   TPT29555();
   TPT29555(TwoWire *w);
   TPT29555(uint8_t addr);
   TPT29555(TwoWire *w, uint8_t addr);

   bool begin(void);
   bool scan();

   void writeReg(uint8_t regAddr, uint8_t regValue);
   void pinMode(uint8_t pin, uint8_t direction);
   void writeAllGPIO(uint8_t regAddress, uint16_t data);
   void writeGPIO(uint8_t regAddress, uint8_t data);
   void digitalWrite(uint8_t pin, uint8_t io_data);
   void writeAllGPIO(uint16_t data);
   uint8_t readGPIO(uint8_t regAddress);
   uint8_t readReg(uint8_t regAddr);
   uint8_t digitalRead(uint8_t port0_address,uint8_t port1_address, uint8_t pin);
   uint8_t digitalRead(uint8_t pin);
   uint16_t readAllGPIO(uint8_t regAddress);
   uint16_t readAllPin(void);
private:
   TwoWire *_wire;
   uint8_t i2cAddress;
   uint8_t bitForPin(uint8_t pin);
   uint8_t pin_bit(uint8_t pin);
   uint8_t pin_register(uint8_t pin, uint8_t port0_address, uint8_t port1_address);
   void updateRegisterBit(uint8_t pin, uint8_t pValue, uint8_t port0_address, uint8_t port1_address);

   void readRegister(uint8_t registerAddress, uint8_t *readData, uint8_t size); 
   void writeRegister(uint8_t registerAddress, uint8_t *writeData, uint8_t size);
};

#endif
