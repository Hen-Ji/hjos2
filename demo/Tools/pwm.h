/*
    每个定时器和通道对应的PWM输出口：
          ch1  ch2  ch3  ch4
    tim2  PA0  PA1  PA2  PA3
    tim3  PA6  PA7  PB0  PB1
    tim4  PB6  PB7  PB8  PB9
*/

#ifndef __PWM_H
#define __PWM_H

#include "global.h"
#include "timer.h"
#include "io.h"

typedef struct _Pwm {
    Timer timer; //定时器
    Io io; //io口
    uint8_t channel; //通道
    uint16_t cmp; //比较值CCR
}Pwm;

/**
  * @brief  PWM初始化
  * @param  pwm Pwm结构体指针
  * @param  tim 使用的定时器(2-4)
  * @param  channel 使用的通道(1-4)
  * @param  period 定时器的重载值
  * @param  prescaler 定时器的预分频系数
  */
void pwmInit(Pwm *pwm, uint8_t tim, uint8_t channel, uint16_t period, uint16_t prescaler);

/**
  * @brief  创建Pwm结构体并初始化
  * @param  tim 使用的定时器(2-4)
  * @param  channel 使用的通道(1-4)
  * @param  period 定时器的重载值
  * @param  prescaler 定时器的预分频系数
  * @retval Pwm结构体
  */
Pwm pwmCreate(uint8_t tim, uint8_t channel, uint16_t period, uint16_t prescaler);

/**
  * @brief  开始生成PWM波
  * @param  pwm Pwm结构体指针
  */
void pwmStart(Pwm *pwm);

/**
  * @brief  设置此PWM定时器的CCR值
  * @param  pwm Pwm结构体指针
  * @param  cmp 定时器的CCR值（在重载值是99的情况下可认为是PWM的占空比）
  */
void pwmSet(Pwm *pwm, uint16_t cmp);

/**
  * @brief  停止生成PWM波
  * @param  pwm Pwm结构体指针
  */
void pwmStop(Pwm *pwm);

#endif