/**
   @file SoftwareIIC.h
   @author rakwireless.com
   @brief Use software to simulate IIC communication.
   @version 0.1
   @date 2021-11-08
   @copyright Copyright (c) 2021
**/

#ifndef __SOFT_WARE_IIC_H__
#define __SOFT_WARE_IIC_H__

#include <arduino.h>
#include "stdint.h"

#define DELYA       5   // Define clock delay.
#define SDA_OUT     pinMode(_sda, OUTPUT)
#define SDA_IN      pinMode(_sda, INPUT)
#define READ_SDA    digitalRead(_sda)
#define IIC_SDA_H   digitalWrite(_sda, HIGH)
#define IIC_SDA_L   digitalWrite(_sda, LOW)
#define IIC_SCL_H   digitalWrite(_scl, HIGH)
#define IIC_SCL_L   digitalWrite(_scl, LOW)

typedef enum
{
  NACK = 0,
  ACK   = 1
} RW_Status_t;

class IOIIC
{
public:
  // IOIIC(uint8_t sda, uint8_t scl);
  void IIC_Init(uint8_t address);
  void IIC_Start(void);
  void IIC_Stop(void);
  uint8_t IIC_Wait_Ack(void);
  void IIC_Ack(void);
  void IIC_NAck(void);
  void IIC_Send_Byte(uint8_t txd);
  uint8_t IIC_Read_Byte(unsigned char ack);

  void set_pin(uint8_t sda, uint8_t scl);
  bool begin(uint8_t address);
//  RW_Status_t ReadOneByte(uint8_t ReadAddr ,uint8_t *Date);
//  RW_Status_t WriteOneByte(uint8_t WriteAddr,uint8_t DataToWrite);

private:
  uint8_t _sda;
  uint8_t _scl;
  uint8_t _address;
  uint8_t _writeAddress;
  uint8_t _readAddress;
};

#endif
