#include "tpt29555.h"
#include "Arduino.h"
#include "SoftwareIIC.h"

IOIIC i2c;

/**
 * @brief Construct a new rak ioexpander tpt29555::rak ioexpander tpt29555 object
 * @param addr the i2c address
 */
TPT29555::TPT29555(uint8_t addr)
{
  i2cAddress = addr;
  #if defined(_VARIANT_RAK11200_)

  #else
  _wire = &Wire; 
  #endif
  
}

/**
 * @brief Construct a new rak ioexpander tpt29555::rak ioexpander tpt29555 object
 *
 */
TPT29555::TPT29555()
{
  i2cAddress = IO_EXPANDER_TPT29555_ADDRESS;
  #if defined(_VARIANT_RAK11200_)

  #else
  _wire = &Wire;  
  #endif  
}

/**
 * @brief Construct a new rak ioexpander tpt29555::rak ioexpander tpt29555 object
 * @param w    i2c channel selection
 * @param addr the i2c address
 */
TPT29555::TPT29555(TwoWire *w, uint8_t addr)
{
  _wire = w;
  i2cAddress = addr;
}

/**
 * @brief Construct a new rak ioexpander tpt29555::rak ioexpander tpt29555 object
 * @param w i2c channel selection
 */
TPT29555::TPT29555(TwoWire *w)
{
  _wire = w;
  i2cAddress = IO_EXPANDER_TPT29555_ADDRESS;
}

/**
 * @brief Bit number associated to a give Pin
 * @param pin  the gpio pin
 * @return     pin
 */
uint8_t TPT29555::pin_bit(uint8_t pin) { return pin % 8; }

/**
 * @brief Register address, port dependent, for a given PIN
 * @param pin             the gpio pin
 * @param port0_address   the port0 register address
 * @param port1_address   the port1 register address
 * @return                the register address
 */
uint8_t TPT29555::pin_register(uint8_t pin, uint8_t port0_address,
                                              uint8_t port1_address)
{
  return (pin < 8) ? port0_address : port1_address;
}

/**
 * @brief Reads the register
 * @param addr  register address
 * @return      the register value
 */
uint8_t TPT29555::readReg(uint8_t regAddr)
{  
  #if defined(_VARIANT_RAK11200_)
  uint8_t read;
  readRegister((uint8_t)regAddr, &read,1);  
  return  read;
  #else
  _wire->beginTransmission((uint8_t)i2cAddress);
  _wire->write((uint8_t)regAddr);
  _wire->endTransmission();
  _wire->requestFrom((uint8_t)i2cAddress, 1);
  return _wire->read();
  #endif
}

/**
 * @brief Writes the register
 * @param regAddr  register address
 */
void TPT29555::writeReg(uint8_t regAddr, uint8_t regValue)
{
  #if defined(_VARIANT_RAK11200_)
  uint8_t write=regValue;
  writeRegister(regAddr, &write, 1);
  #else
  // Write the register
  _wire->beginTransmission((uint8_t)i2cAddress);
  _wire->write((uint8_t)regAddr);
  _wire->write((uint8_t)regValue);
  _wire->endTransmission();
  #endif
}

/**
 * @brief update a single bit of an register.
 * @param pin              the update I/O pin
 * @param pValue           value to be updated
 * @param port0_address    register address
 * @param port1_address    register address
 */
void TPT29555::updateRegisterBit(uint8_t pin, uint8_t pValue,
                                                uint8_t port0_address,
                                                uint8_t port1_address)
{
  uint8_t regValue;
  uint8_t regAddr = pin_register(pin, port0_address, port1_address);
  uint8_t bit = pin_bit(pin);
  regValue = readReg(regAddr);
  // Serial.printf("pValue=%x--regAddr=%x--regValue=%x\r\n", pValue,regAddr,regValue);
  // set the value for the particular bit

  bitWrite(regValue, bit, pValue);

  writeReg(regAddr, regValue);
}

/**
 @fn scan
 @brief  iic scan function
 @return scan result
 @retval true sensor exist
 @retval false There is no sensor
*/
bool TPT29555::scan()
{
  _wire->beginTransmission(i2cAddress);
  if (_wire->endTransmission() == 0)
  {
    return true;
  }
  return false;
}

/**
   @brief set the i2c addr and init the i2c.
   @return returns the initialization status
   @retval true Initialization succeeded
   @retval false Initialization  failed
*/
bool TPT29555::begin(void)
{
#if defined(_VARIANT_RAK11200_)
  i2c.set_pin(SDA,SCL);
  return(i2c.begin(i2cAddress));
#else
  _wire->begin();
  if (scan() == true)
  {
    // set defaults!
    // all inputs on port0 an port1 (0~7)
    writeReg(TPT29555_CONFIGURATION_PORT0_REGISTER, 0xff);
    writeReg(TPT29555_CONFIGURATION_PORT1_REGISTER, 0xff);
    return true;
  }
  return false;
#endif

}

/**
 * @brief  Sets the pin mode to either INPUT or OUTPUT
 * @param pin Pin to set
 * @param direction  Mode to set the pin
 */
void TPT29555::pinMode(uint8_t pin, uint8_t direction)
{
  updateRegisterBit(pin, (direction == INPUT), TPT29555_CONFIGURATION_PORT0_REGISTER, TPT29555_CONFIGURATION_PORT1_REGISTER);
}

/**
 * @brief Reads all 16 pins (port 1 and 2) into a single 16 bits variable
 * @param regAddress Register address
 * @return Returns the 16 bit variable representing all 16 pins
 */
uint16_t TPT29555::readAllGPIO(uint8_t regAddress)
{
  uint16_t data = 0;
  uint8_t a;

  #if defined(_VARIANT_RAK11200_)
  uint8_t temp[2]={0};
  readRegister(regAddress,temp,2);
  data = temp[0];
  data = (data<<8)|temp[1];
  return data;  
  #else
  _wire->beginTransmission(i2cAddress);
  _wire->write(regAddress);
  _wire->endTransmission();

  _wire->requestFrom(i2cAddress, 2);
  a = _wire->read();
  data = _wire->read();
  data <<= 8;
  data |= a;
  return data;
  #endif
}

/**
 * @brief Writes all 16 pins (port 1 and 2)
 * @param regAddress Register address
 * @param data       The value you want to write(16bit)
 */
void TPT29555::writeAllGPIO(uint8_t regAddress, uint16_t data)
{
  #if defined(_VARIANT_RAK11200_)
  uint8_t temp[2]={0};
  temp[0]=uint8_t((data>>8)&0xff);
  temp[1]=uint8_t(data&0xff);
  writeRegister(regAddress,temp,2);
  #else
  _wire->beginTransmission(i2cAddress);
  _wire->write(regAddress);
  _wire->write(data & 0xFF);
  _wire->write(data >> 8);
  _wire->endTransmission();
  #endif
}

/**
 * @brief Read a single port, port0 or port1, and return its current 8 bit value
 * @param regAddress Register address
 * @return Returns the 8 bit value of the port
 */
uint8_t TPT29555::readGPIO(uint8_t regAddress)
{
  #if defined(_VARIANT_RAK11200_)
  uint8_t temp=0;
  readRegister(regAddress,&temp,1);
  return temp;  
  #else
  // read the current GPIO output latches
  _wire->beginTransmission(i2cAddress);
  _wire->write(regAddress);
  _wire->endTransmission();
  _wire->requestFrom(i2cAddress, 1);
  return _wire->read();
  #endif
}
uint16_t TPT29555::readAllPin(void)
{
 uint16_t state = readGPIO(1);
 state = (state<<8)|readGPIO(0);
 return state;
}
/**
 * @brief Writes 8 pins (port 1 or 2)
 * @param regAddress Register address
 * @param data       The value you want to write(8bit)
 */
void TPT29555::writeGPIO(uint8_t regAddress, uint8_t data)
{
  #if defined(_VARIANT_RAK11200_)
    uint8_t write=data;
    writeRegister(regAddress, &write, 1);
  #else
  _wire->beginTransmission(i2cAddress);
  _wire->write(regAddress);
  _wire->write(data);
  _wire->endTransmission();
  #endif
}
/**
 * @brief Writes all 16 pins (port 1 and 2)
 * @param data       The value you want to write(16bit)
 */
void TPT29555::writeAllGPIO(uint16_t data)
{
  writeGPIO(TPT29555_OUTPUT_PORT0_REGISTER,(uint8_t)(data&0xff));
  writeGPIO(TPT29555_OUTPUT_PORT1_REGISTER,(uint8_t)((data>>8)&0xff));
}
/**
 * @brief Writes to a pin on the TPT29555
 * @param regAddress  Register address
 * @param pin         The pin to be written
 * @param io_data     What to write to the pin
 */
void TPT29555::digitalWrite(uint8_t pin, uint8_t io_data)
{
  uint8_t gpio;
  uint8_t bit = pin_bit(pin);

  // read the current GPIO output latches
  uint8_t regAddr = pin_register(pin, TPT29555_OUTPUT_PORT0_REGISTER, TPT29555_OUTPUT_PORT1_REGISTER);
  gpio = readReg(regAddr);

  // set the pin and direction
  bitWrite(gpio, bit, io_data);

  // write the new GPIO
  regAddr = pin_register(pin, TPT29555_OUTPUT_PORT0_REGISTER, TPT29555_OUTPUT_PORT1_REGISTER);
  writeReg(regAddr, gpio);
}

/**
 * @brief Reads the specified pin
 * @param pin         Pin to read
 * @return            Value of the pin
 */
uint8_t TPT29555::digitalRead(uint8_t port0_address,uint8_t port1_address, uint8_t pin)
{
  uint8_t bit = pin_bit(pin);
  uint8_t regAddr = pin_register(pin, port0_address, port1_address);
  return (readReg(regAddr) >> bit) & 0x1;
}

/**
 * @brief Reads the specified pin
 * @param pin         Pin to read
 * @return            Value of the pin
 */
uint8_t TPT29555::digitalRead(uint8_t pin)
{
  uint8_t bit = pin_bit(pin);
  uint8_t regAddr = pin_register(pin, TPT29555_INPUT_PORT0_REGISTER, TPT29555_INPUT_PORT1_REGISTER);
  return (readReg(regAddr) >> bit) & 0x1;
}

/*!
 *  @brief  I2c bus write.
 *  @param  registerAddress : Register address.
 *  @param  writeData		    : Write data pointer.
 *  @param  size			      : The length of the written data.
 */
void TPT29555::writeRegister(uint8_t registerAddress, uint8_t *writeData, uint8_t size)
{
  i2c.IIC_Start();  
  i2c.IIC_Send_Byte( (i2cAddress<<1)&0xFE);
  i2c.IIC_Wait_Ack();
//  if(i2c.IIC_Wait_Ack())
//    return NACK;    
  i2c.IIC_Send_Byte( registerAddress );  
  i2c.IIC_Wait_Ack();     
//  if(i2c.IIC_Wait_Ack())
//    return NACK;
  for(uint8_t i = 0; i < size; i++)
  {      
    i2c.IIC_Send_Byte( writeData[i] ); 
    i2c.IIC_Wait_Ack();   
  }   
//  if(i2c.IIC_Wait_Ack())
//    return NACK;
  i2c.IIC_Stop();
//  return ACK; 

}

/*!
 *  @brief  I2c bus read.
 *  @param  registerAddress : Register address.
 *  @param  readData		    : Read data pointer.
 *  @param  size			      : The length of the written data.
 */
void TPT29555::readRegister(uint8_t registerAddress, uint8_t *readData, uint8_t size) 
{ 
  i2c.IIC_Start();  
  i2c.IIC_Send_Byte( (i2cAddress<<1)&0xfe );
  i2c.IIC_Wait_Ack();
//  if(i2c.IIC_Wait_Ack())
//    return NACK;
  i2c.IIC_Send_Byte( registerAddress );   
  i2c.IIC_Wait_Ack();   
//  if(i2c.IIC_Wait_Ack())
//    return NACK;
  i2c.IIC_Stop();
  delayMicroseconds(DELYA); 
  i2c.IIC_Start();
  i2c.IIC_Send_Byte( (i2cAddress<<1)|0x01 );
  i2c.IIC_Wait_Ack();
//  if(i2c.IIC_Wait_Ack())
//    return NACK; 
    
  for(uint8_t i = 0; i < size; i++)
  {    
    readData[i] = i2c.IIC_Read_Byte(0);
  }        
  i2c.IIC_Stop();

}