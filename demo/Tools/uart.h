/* USART引脚定义
          USART1  USART2  USART3
    TXD    PA9     PA2     PB10
    RXD    PA10    PA3     PB11
*/

#ifndef __UART_H
#define __UART_H

#include "global.h"
#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct _Uart {
    Io tx, rx; //tx，rx引脚
    USART_TypeDef * handle; //USART的TypeDef
    uint32_t baud; //波特率
    uint16_t parity; //校验位
    uint16_t stopBits; //停止位
    uint16_t dataBits; //数据位
    uint8_t id; //序号
}Uart;

/**
  * @brief  硬件UART初始化
  * @param  uart Uart结构体指针
  * @param  num 使用的USART(1-3)
  * @param  baud 波特率，为 0 则默认使用 115200 波特率
  */
void uartInitHardware(Uart* uart, uint8_t num, uint32_t baud);

/**
  * @brief  硬件UART初始化并返回对应结构体
  * @param  num 使用的USART(1-3)
  * @param  baud 波特率，为 0 则默认使用 115200 波特率
  * @retval Uart结构体
  */
Uart uartCreateHardware(uint8_t num, uint32_t baud);

/**
  * @brief  UART发送一个字节数据
  * @param  uart Uart结构体指针
  * @param  byte 一个字节的数据
  */
void uartTransmitByte(Uart* uart, uint8_t byte);

/**
  * @brief  UART发送数据
  * @param  uart Uart结构体指针
  * @param  data 数据指针
  * @param  size 数据长度
  */
void uartTransmit(Uart* uart, uint8_t* data, uint32_t size);

/**
  * @brief  UART打印字符串(发送格式化字符串)
  * @param  uart Uart结构体指针
  * @param  format 格式化字符串
  * @param  ... 不定参数
  */
void uartPrint(Uart* uart, const char* const format, ...);

/**
  * @brief  UART设置接收中断函数的回调函数
  * @param  uart Uart结构体指针
  * @param  callback 一个回调函数，当接收到一个字节的数据时，会将这个数据传入callback
  * @param  prePriority 抢占优先级
  * @param  subPriority 响应优先级
  */
void uartSetReceiveCb(Uart* uart, void (*callback)(uint8_t), uint8_t prePriority, uint8_t subPriority);

/**
  * @brief  UART使用内置的接收函数回调
  * @param  uart Uart结构体指针
  * @param  callback 一个回调函数，当接收到一串完整的字符串 (即以\0结尾的字符串) , 或这个字符串的长度达到最大长度（127）时，会将这个字符串的指针和长度传入callback
  * @param  prePriority 抢占优先级
  * @param  subPriority 响应优先级
  */
void uartUseDefaultCb(Uart* uart, void (*callback)(char*, uint32_t), uint8_t prePriority, uint8_t subPriority);
#endif