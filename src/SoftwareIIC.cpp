/**
   @file SoftwareIIC.cpp
   @author rakwireless.com
   @brief Use software to simulate IIC communication.
   @version 0.1
   @date 2021-11-08
   @copyright Copyright (c) 2021
**/

#include "SoftwareIIC.h"

/*!
 *  @brief  Initialize the IIC IO.
 */
void IOIIC::IIC_Init(uint8_t address)
{
  _address = address;
  pinMode(_sda, OUTPUT);
  pinMode(_scl, OUTPUT);
  
  IIC_SDA_H;
  IIC_SCL_H;
  
  delayMicroseconds(DELYA);
}
void IOIIC::set_pin(uint8_t sda, uint8_t scl)
{
  _sda = sda;
  _scl = scl;
}
bool IOIIC::begin(uint8_t address)
{
  _address = address;
  pinMode(_sda, OUTPUT);
  pinMode(_scl, OUTPUT);  
  IIC_SDA_H;
  IIC_SCL_H;  
  delay(20);
  // delayMicroseconds(100); // delayMicroseconds(DELYA);
  // Serial.println("###### SOFTWARE IIC INIT #####");
  IIC_Start();
  IIC_Send_Byte((address<<1));  
 if(IIC_Wait_Ack())   return false;   

  return true;

}
/*!
 *  @brief  Generate IIC start signal.
 */
void IOIIC::IIC_Start(void)
{
  /*    _____
  *SDA      \_____________
  *    __________
  *SCL           \________
  */
  SDA_OUT;
  delayMicroseconds(DELYA);
  IIC_SDA_H;        
  IIC_SCL_H;
  delayMicroseconds(DELYA);
  IIC_SDA_L;  //  START:when CLK is high,DATA change form high to low. 
  delayMicroseconds(DELYA);
  IIC_SCL_L;
}   

/*!
 *  @brief  Generate IIC stop signal.
 */
void IOIIC::IIC_Stop(void)
{
  /*               _______
   *SDA __________/
   *          ____________
   *SCL _____/
   */
  SDA_OUT;
  IIC_SCL_L;
  IIC_SDA_L;  //  STOP:when CLK is high DATA change form low to high.
  delayMicroseconds(DELYA);
  IIC_SCL_H;   
  IIC_SDA_H; 
  delayMicroseconds(DELYA);    
}

/*!
 *  @brief  Waiting for the arrival of the response signal.
 *  @return 1 : ACK
 *          0 : NACK
 */
uint8_t IOIIC::IIC_Wait_Ack(void)
{
  uint8_t ucErrTime=0;
  SDA_IN;   //  SDA is set as input.
  IIC_SDA_H;
  delayMicroseconds(DELYA);     
  IIC_SCL_H;
  delayMicroseconds(DELYA);   
  while(READ_SDA == HIGH)
  {
    ucErrTime++;
    if(ucErrTime > 250)
    {
      IIC_Stop();
      return 1; // Time Out 
    }
  }
  IIC_SCL_L;
  //delayMicroseconds(DELYA);   
  //IIC_Stop();
  return 0;  
} 

/*!
 *  @brief  Generate IIC ACK signal.
 */
void IOIIC::IIC_Ack(void)
{
  /*           ____
   *SCL ______/    \______
   *    ____         _____
   *SDA     \_______/
   */
  IIC_SCL_L;
  SDA_OUT;
  IIC_SDA_L;
  delayMicroseconds(DELYA);
  IIC_SCL_H;
  delayMicroseconds(DELYA);
  IIC_SCL_L;
  // delayMicroseconds(DELYA);
  // IIC_SDA_H;
}

/*!
 *  @brief  Generate IIC NACK signal.
 */
void IOIIC::IIC_NAck(void)
{
  /*           ____
   *SCL ______/    \______
   *    __________________
   *SDA
   */
  IIC_SCL_L;
  SDA_OUT;
  IIC_SDA_H;
  delayMicroseconds(DELYA);
  IIC_SCL_H;
  delayMicroseconds(DELYA);
  IIC_SCL_L;
  // delayMicroseconds(DELYA);
  // IIC_SDA_H;
}                      

/*!
 *  @brief  Send one byte of data through IIC.
 *  @param  txd : Data sent. 
 */       
void IOIIC::IIC_Send_Byte(uint8_t txd)
{                        
  uint8_t t;   
  SDA_OUT;      
  IIC_SCL_L; // Pull down the clock to start data transmission.
  for(t = 0 ; t < 8 ; t++)
  {              
    //IIC_SDA=(txd&0x80)>>7;
    if(((txd&0x80)>>7) == 1)
    {
      IIC_SDA_H;
    }
    else 
    {
      IIC_SDA_L;
    }
    txd<<=1;    
    delayMicroseconds(DELYA+1);   
    IIC_SCL_H;
    delayMicroseconds(DELYA+1); 
    IIC_SCL_L;  
    delayMicroseconds(DELYA+1);
  }  
}
 
/*!
 *  @brief  Read one byte of data through IIC.
 *  @param  ack : 0:NACK , 1:ACK. 
 */  
uint8_t IOIIC::IIC_Read_Byte(unsigned char ack)
{
  unsigned char i,date=0;
  SDA_IN;      // SDA pin is set as input.
  for(i=0;i<8;i++ )
  {
    IIC_SCL_L; 
    delayMicroseconds(DELYA*2);
    IIC_SCL_H;
    date<<=1;
    if(READ_SDA == HIGH)
    {
      date++;   
    }
    delayMicroseconds(DELYA*2); 
  }        

  if (!ack)
    IIC_NAck();
  else
    IIC_Ack(); 
 
  return date;
}
