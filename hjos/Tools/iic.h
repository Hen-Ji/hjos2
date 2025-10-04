/* I2C引脚定义
          I2C1    I2C2
    SCL   PB6     PB10
    SDA   PB7     PB11
*/

#ifndef __IIC_H
#define __IIC_H

#include "global.h"
#include "io.h"

#define IIC_WRITE 0
#define IIC_READ 1
#define IIC_ACK 0
#define IIC_NACK 1

typedef struct _IicBus { //IIC总线
    uint8_t isHardware; //是否使用硬件IIC
    uint8_t busy; //总线是否处于忙状态
    uint8_t ack; //保存ack
    I2C_TypeDef * handle; //IIC的TypeDef
    Io scl, sda; //IIC的scl, sda引脚
    uint32_t freq; //IIC传输速率
    uint32_t cycle; //每改变一次电平状态后加上的延时时间
}IicBus;

typedef struct _IicDevice { //挂载在IIC总线上的设备
    uint8_t address; //7位设备地址（存低7位上）
    IicBus* bus; //挂载在的总线
}IicDevice;

/**
  * @brief  硬件IIC初始化
  * @param  bus IicBus结构体指针
  * @param  num 使用的IIC(1-2)
  * @param  freq IIC传输频率，为0默认为100kB/s（建议不大于400 000）
  */
void iicInitHardware(IicBus* bus, uint8_t num, uint32_t freq);

/**
  * @brief  软件IIC初始化
  * @param  bus IicBus结构体指针
  * @param  sclPort scl端口号
  * @param  sclPin scl引脚号
  * @param  sdaPort sda端口号
  * @param  sdaPin sda引脚号
  * @param  cycle scl和sda电平变化后的延时，值越大总线传输速率越低
  */
void iicInitSoftware(IicBus* bus, uint8_t sclPort, uint8_t sclPin, uint8_t sdaPort, uint8_t sdaPin, uint32_t cycle);

/**
  * @brief  硬件IIC初始化并返回IIC总线结构体
  * @param  num 使用的IIC(1-2)
  * @param  freq IIC传输频率，为0默认为100kB/s（建议不大于400 000）
  */
IicBus iicCreateHardware(uint8_t num, uint32_t freq);

/**
  * @brief  软件IIC初始化并返回IIC总线结构体
  * @param  sclPort scl端口号
  * @param  sclPin scl引脚号
  * @param  sdaPort sda端口号
  * @param  sdaPin sda引脚号
  * @param  cycle scl和sda电平变化后的延时，值越大总线传输速率越低
  */
IicBus iicCreateSoftware(uint8_t sclPort, uint8_t sclPin, uint8_t sdaPort, uint8_t sdaPin, uint32_t cycle);

/**
  * @brief  IIC挂载设备
  * @param  device IicDevice结构体指针
  * @param  bus 要挂载到的总线的结构体指针
  * @param  address 设备地址（存低7位上）
  */
void iicAddDevice(IicDevice* device, IicBus* bus, uint8_t address);

/**
  * @brief  IIC挂载设备并返回IicDevice结构体
  * @param  bus 要挂载到的总线的结构体指针
  * @param  address 设备地址（存低7位上）
  * @retval IicDevice结构体
  */
IicDevice iicCreateDevice(IicBus* bus, uint8_t address);

/**
  * @brief  IIC发送数据
  * @param  device 希望操作的设备的结构体指针
  * @param  data 数据指针
  * @param  size 数据长度
  * @retval 返回0表示正常；返回1表示总线在忙，暂时不可用
  */
uint8_t iicTransmit(IicDevice* device, uint8_t* data, uint32_t size);

/**
  * @brief  IIC接收数据
  * @param  device 希望操作的设备的结构体指针
  * @param  data 希望发送出去的数据的指针
  * @param  size 希望发送出去的数据的长度
  * @param  receiveBuffer 读出来的数据的存放位置
  * @param  receiveSize 希望读的数据长度
  * @retval 返回0表示正常；返回1表示总线在忙，暂时不可用
  */
uint8_t iicReceive(IicDevice* device, uint8_t* data, uint32_t size, uint8_t* receiveBuffer, uint32_t receiveSize);

/************注：上面的两个函数不能满足要求的时可以使用下面的函数************/

/**
  * @brief  IIC总线启动
  * @param  bus IicBus结构体指针
  * @retval 返回0表示正常；返回1表示总线在忙，暂时不可用
  */
uint8_t iicBusStart(IicBus* bus);

/**
  * @brief  IIC总线重启
  * @param  bus IicBus结构体指针
  */
uint8_t iicBusRestart(IicBus* bus);

/**
  * @brief  IIC总线停止
  * @param  bus IicBus结构体指针
  */
uint8_t iicBusStop(IicBus* bus);

/**
  * @brief  IIC总线发送一个字节
  * @param  bus IicBus结构体指针
  * @param  byte 要发送的字节
  */
uint8_t iicBusTransmitByte(IicBus* bus, uint8_t byte);

/**
  * @brief  IIC总线发送数据
  * @param  bus IicBus结构体指针
  * @param  data 数据指针
  * @param  size 数据长度
  */
uint8_t iicBusTransmit(IicBus* bus, uint8_t* data, uint32_t size);

/**
  * @brief  IIC总线发送地址
  * @param  bus IicBus结构体指针
  * @param  addr 设备地址
  * @param  direction IIC_WRITE 为写模式，IIC_READ 为读模式
  */
uint8_t iicBusTransmitAddr(IicBus* bus, uint8_t addr, uint8_t direction);

/**
  * @brief  IIC总线接收一个字节
  * @param  bus IicBus结构体指针
  * @param  byte 字节的存放位置
  * @param  ack IIC_ACK 为应答，表示还要从机继续传数据，IIC_NACK 为非应答，表示传输结束
  */
uint8_t iicBusReceiveByte(IicBus* bus, uint8_t* byte, uint8_t ack);

/**
  * @brief  IIC总线接收数据
  * @param  bus IicBus结构体指针
  * @param  data 数据指针
  * @param  size 数据长度
  */
uint8_t iicBusReceive(IicBus* bus, uint8_t* data, uint32_t size);

/**
  * @brief  IIC总线读上个数据发送出去后的应答 (注：若使用硬件IIC，则此函数永远返回0)
  * @param  bus IicBus结构体指针
  * @retval 应答信号
  */
uint8_t iicBusReadAck(IicBus* bus);

#endif
