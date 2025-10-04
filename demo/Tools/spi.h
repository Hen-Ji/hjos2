/* SPI引脚定义
            SPI1    SPI2
    MISO    PA6     PB14
    MOSI    PA7     PB15
    SCLK    PA5     PB13
*/

#ifndef __SPI_H
#define __SPI_H

#include "global.h"
#include "io.h"
#include "delay.h"

typedef struct _SpiBus {
	uint8_t isHardWare; //是否是硬件SPI
    uint8_t busy; //总线是否忙
    SPI_TypeDef* handle; //SPI的handle
    Io miso, mosi, sclk; //MISO, MOSI, SCLK引脚
    uint32_t freq; //SPI总线频率
    uint32_t cycle; //SPI延时时间
}SpiBus;

typedef struct _SpiDevice {
    SpiBus* bus; //挂载到的总线
    Io cs; //CS引脚
}SpiDevice;

/**
  * @brief  硬件SPI初始化
  * @param  bus SpiBus结构体指针
  * @param  num 使用的Spi(1-2)
  * @param  freq Spi传输速率（单位: bits/s），为0默认为最高速率（SPI1的最高速率为 36M bits/s）
  */
void spiInitHardware(SpiBus *bus, uint8_t num, uint32_t freq);

/**
  * @brief  硬件SPI初始化并返回对应结构体
  * @param  num 使用的Spi(1-2)
  * @param  freq Spi传输速率（单位: bits/s），为0默认为最高速率（SPI1的最高速率为 36M bits/s）
  * @retval SpiBus结构体
  */
SpiBus spiCreateHardware(uint8_t num, uint32_t freq);

/**
  * @brief  软件SPI初始化
  * @param  bus SpiBus结构体指针
  * @param  misoPort MISO端口号
  * @param  misoPin MISO引脚号
  * @param  mosiPort MOSI端口号
  * @param  mosiPin MOSI引脚号
  * @param  sclkPort SCLK端口号
  * @param  sclkPin SCLK引脚号
  * @param  cycle SCLK和MOSI电平变化后的延时，值越大总线传输速率越低
  */
void spiInitSoftware(SpiBus *bus, uint8_t misoPort, uint8_t misoPin, uint8_t mosiPort, uint8_t mosiPin, uint8_t sclkPort, uint8_t sclkPin, uint32_t cycle);

/**
  * @brief  软件SPI初始化并返回对应结构体
  * @param  misoPort MISO端口号
  * @param  misoPin MISO引脚号
  * @param  mosiPort MOSI端口号
  * @param  mosiPin MOSI引脚号
  * @param  sclkPort SCLK端口号
  * @param  sclkPin SCLK引脚号
  * @param  cycle SCLK和MOSI电平变化后的延时，值越大总线传输速率越低
  * @retval SpiBus结构体
  */
SpiBus spiCreateSoftware(uint8_t misoPort, uint8_t misoPin, uint8_t mosiPort, uint8_t mosiPin, uint8_t sclkPort, uint8_t sclkPin, uint32_t cycle);

/**
  * @brief  SPI挂载设备
  * @param  device SpiDevice结构体指针
  * @param  bus 要挂载到的总线的结构体指针
  * @param  csPort CS端口号
  * @param  csPin CS引脚号
  */
void spiAddDevice(SpiDevice* device, SpiBus* bus, uint8_t csPort, uint8_t csPin);

/**
  * @brief  SPI挂载设备并返回对应结构体
  * @param  bus 要挂载到的总线的结构体指针
  * @param  csPort CS端口号
  * @param  csPin CS引脚号
  * @retval SpiDevice结构体
  */
SpiDevice spiCreateDevice(SpiBus* bus, uint8_t csPort, uint8_t csPin);

/**
  * @brief  SPI发送数据
  * @param  device 设备的结构体指针
  * @param  data 要发送的数据指针
  * @param  size 发送数据的长度
  * @retval 返回1表示总线已被占用
  */
uint8_t spiTransmit(SpiDevice* device, uint8_t* data, uint32_t size);

/**
  * @brief  SPI接收数据
  * @param  device 设备的结构体指针
  * @param  data 希望发送出去的数据的指针
  * @param  size 希望发送出去的数据的长度
  * @param  receiveBuffer 读出来的数据的存放位置
  * @param  receiveSize 希望读的数据长度
  * @retval 返回1表示总线已被占用
  */
uint8_t spiReceive(SpiDevice* device, uint8_t* data, uint32_t size, uint8_t* receiveBuffer, uint32_t receiveSize);

/**
  * @brief  SPI交换数据
  * @param  device 设备的结构体指针
  * @param  data 要交换的数据指针，为0表示只发送0xFF
  * @param  receiveBuffer 接收到的数据的存放位置，为0表示不接收数据
  * @param  size 交换数据的长度
  * @retval 返回1表示总线已被占用
  */
uint8_t spiSwap(SpiDevice* device, uint8_t* data, uint8_t* receiveBuffer, uint32_t size);

/************注：上面的函数不能满足要求的时可以使用下面的函数************/

/**
  * @brief  SPI总线启动
  * @param  bus 总线的结构体指针
  * @param  cs 要访问的设备的CS引脚
  * @retval 返回0表示正常启动，返回1表示总线已被占用
  */
uint8_t spiBusStart(SpiBus* bus, Io* cs);

/**
  * @brief  SPI总线停止
  * @param  bus 总线的结构体指针
  * @param  cs 要停止访问的设备的CS引脚
  * @retval 返回0表示正常停止，返回1表示总线未被使用
  */
uint8_t spiBusStop(SpiBus* bus, Io* cs);

/**
  * @brief  SPI总线交换一个字节
  * @param  bus 总线的结构体指针
  * @param  byte 要传输的字节
  * @retval 返回接收到的一个字节
  */
uint8_t spiBusSwapByte(SpiBus* bus, uint8_t byte);

/**
  * @brief  SPI总线交换数据
  * @param  bus 总线的结构体指针
  * @param  data 要交换的数据指针，为0表示只发送0xFF
  * @param  receiveBuffer 接收到的数据的存放位置，为0表示不接收数据
  * @param  size 交换数据的长度
  * @retval 返回交换出来的数据
  */
void spiBusSwap(SpiBus* bus, uint8_t* data, uint8_t* receiveBuffer, uint32_t size);

#endif