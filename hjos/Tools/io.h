//输入输出引脚推荐:
//PA0-PA12, PB0, PB1, PB5-PB12

#ifndef __IO_H
#define __IO_H

#include "global.h"

typedef struct _Io {
	GPIO_TypeDef* port;
	uint16_t pin;
	uint8_t portId;
	uint8_t pinId;
	GPIOMode_TypeDef mode;
}Io;

#define IO_AIN 0 //模拟输入
#define IO_FL 1 //浮空输入
#define IO_PD 2 //下拉输入
#define IO_PU 3 //上拉输入
#define IO_OD 4 //开漏输出
#define IO_PP 5 //推挽输出
#define IO_AFOD 6 //复用开漏输出
#define IO_AFPP 7 //复用推挽输出

/**
  * @brief  io初始化
  * @param  io Io结构体指针
  * @param  port io口端口号
  * @param  pin io口引脚号
  * @param  mode io口模式
  */
void ioInit(Io* io, uint8_t port, uint8_t pin, uint8_t mode);

/**
  * @brief  创建Io结构体并初始化
  * @param  port io口端口号
  * @param  pin io口引脚号
  * @param  mode io口模式
  */
Io ioCreate(uint8_t port, uint8_t pin, uint8_t mode);

/**
  * @brief  设置io口电平
  * @param  io Io结构体指针
  * @param  val io口电平
  */
void ioSet(Io* io, uint8_t val);

/**
  * @brief  获取io口电平
  * @param  io Io结构体指针
  * @retval io口电平
  */
uint8_t ioGet(Io* io);

#endif
