#include "global.h"
#include "io.h"
#include "exti.h"
#include "delay.h"
#include "spi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define LORA_STANDBY 0
#define LORA_TX 1
#define LORA_RX 2

#define LORA_BW_125KHZ 0x04
#define LORA_BW_250KHZ 0x05
#define LORA_BW_500KHZ 0x06

#define LORA_SF_5 0x05
#define LORA_SF_6 0x06
#define LORA_SF_7 0x07
#define LORA_SF_8 0x08
#define LORA_SF_9 0x09
#define LORA_SF_10 0x0A
#define LORA_SF_11 0x0B

#define LORA_CR_4_5 0x01
#define LORA_CR_4_6 0x02
#define LORA_CR_4_7 0x03
#define LORA_CR_4_8 0x04

typedef struct _LoraStatus {
    uint8_t mode;
    uint8_t status;
}LoraStatus;

typedef struct _LoraIrqStatus {
    uint8_t txDone;
    uint8_t rxDone;
    uint8_t preambleDetected;
    uint8_t headerValid;
    uint8_t headerErr;
    uint8_t crcErr;
    uint8_t cadDone;
    uint8_t cadDetected;
    uint8_t timeout;
}LoraIrqStatus;

/**
  * @brief  LoRa初始化
  * @param  bus SpiBus结构体指针
  * @param  csPort CS端口号
  * @param  csPin CS引脚号
  * @param  rstPort RESET端口号
  * @param  rstPin RESET引脚号
  * @param  busyPort BUSY端口号
  * @param  busyPin BUSY引脚号
  * @param  dioPort DIO端口号
  * @param  dioPin DIO引脚号
  * @param  dioPrePriority DIO中断抢占优先级
  * @param  dioSubPriority DIO中断响应优先级
  */
void loraInit(SpiBus* bus, uint8_t csPort, uint8_t csPin, uint8_t rstPort, uint8_t rstPin, uint8_t busyPort, uint8_t busyPin, uint8_t dioPort, uint8_t dioPin, uint8_t dioPrePriority, uint8_t dioSubPriority);

/**
  * @brief  LoRa设置射频频率（默认为470.5MHz）
  * @param  freq 射频频率（单位：Hz）
  */
void loraSetFreq(uint32_t freq);

/**
  * @brief  LoRa设置带宽（默认为125KHz）
  * @param  bw 带宽（LORA_BW_125KHZ，LORA_BW_250KHZ，LORA_BW_500KHZ）
  */
void loraSetBandwidth(uint8_t bw);

/**
  * @brief  LoRa设置扩散因子（默认为9）
  * @param  sf 扩散因子（LORA_SF_5到LORA_SF_11）
  */
void loraSetSpreadingFactor(uint8_t sf);

/**
  * @brief  LoRa设置编码率（默认为4/5）
  * @param  cr 编码率（LORA_CR_4_5到LORA_CR_4_8）
  */
void loraSetCodingRate(uint8_t cr);

/**
  * @brief  LoRa设置模式
  * @param  mode LORA_STANDBY：待机模式；LORA_TX：发送模式；LORA_RX：接收模式
  */
void loraSetMode(uint8_t mode);

/**
  * @brief  LoRa设置待机模式
  */
void loraSetStandbyMode();

/**
  * @brief  LoRa设置发送模式
  */
void loraSetTransmitMode();

/**
  * @brief  LoRa设置接收模式
  */
void loraSetReceiveMode();

/**
  * @brief  LoRa设置发送回调
  * @param  p 发送回调，result=0：发送完成；result=1：发送超时
  */
void loraSetTransmitCb(void (*p)(uint8_t result));

/**
  * @brief  LoRa设置接收回调
  * @param  p 接收回调，接收到数据后将数据以及数据长度传入回调
  */
void loraSetReceiveCb(void (*p)(uint8_t* data, uint8_t size));

/**
  * @brief  LoRa异步发送
  * @param  bytes 发送的数据
  * @param  size 发送数据的长度
  * @param  timeout 超时时间，0为禁用超时
  * @retval 0为正常，1为有数据正在发送
  */
uint8_t loraTransmit(uint8_t *bytes, uint8_t size, uint32_t timeout);

/**
  * @brief  LoRa同步发送
  * @param  bytes 发送的数据
  * @param  size 发送数据的长度
  * @param  timeout 超时时间，0为禁用超时
  */
void loraTransmitSync(uint8_t *bytes, uint8_t size, uint32_t timeout);

/**
  * @brief  LoRa异步发送格式化字符串
  * @param  timeout 超时时间，0为禁用超时
  * @param  format 格式化字符串
  * @param  ... 不定参数
  * @retval 0为正常，1为有数据正在发送
  */
uint8_t loraPrint(uint32_t timeout, const char* const format, ...);

/**
  * @brief  LoRa同步发送格式化字符串
  * @param  timeout 超时时间，0为禁用超时
  * @param  format 格式化字符串
  * @param  ... 不定参数
  */
void loraPrintSync(uint32_t timeout, const char* const format, ...);

/**
  * @brief  LoRa是否在忙状态
  * @retval 0为空闲，1为忙
  */
uint8_t loraIsBusy();

/**
  * @brief  LoRa获取DIO1的电平
  * @retval 0为低电平，1为高电平
  */
uint8_t loraGetDio();

/**
  * @brief  LoRa是否正在传输数据
  * @retval 0为空闲，1为正在传输
  */
uint8_t loraIsTransmitting();

/**
  * @brief  LoRa获取设备状态
  * @retval LoraStatus 设备状态结构体
  */
LoraStatus loraGetStatus();

/**
  * @brief  LoRa获取设备中断标志
  * @retval LoraIrqStatus 设备中断标志结构体
  */
LoraIrqStatus loraGetIrqStatus();

/**
  * @brief  LoRa清除设备中断标志位
  * @param mask 需要清除的标志位
  */
void loraClearIrqStatus(uint16_t mask);

/**
  * @brief  LoRa获取设备寄存器的值
  * @param addr 寄存器地址
  * @retval 寄存器值
  */
uint8_t loraReadRegister(uint16_t addr);