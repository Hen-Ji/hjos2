//只支持TIM2, TIM3, TIM4

#ifndef __TIMER_H
#define __TIMER_H

#include "global.h"

typedef struct _Timer {
    TIM_TypeDef * tim;
    uint16_t period;
    uint16_t prescaler;
    uint8_t prePriority;
    uint8_t subPriority;
    uint8_t channel;
    uint8_t id; //定时器id，值为定时器序号减2
}Timer;

/**
  * @brief  定时器初始化
  * @param  timer Timer结构体指针
  * @param  tim 使用的定时器(2-4)
  * @param  period 定时器的重载值
  * @param  prescaler 定时器的预分频系数
  */
void timerInit(Timer* timer, uint8_t tim, uint16_t period, uint16_t prescaler);

/**
  * @brief  创建Timer结构体并初始化
  * @param  tim 使用的定时器(2-4)
  * @param  period 定时器的重载值
  * @param  prescaler 定时器的预分频系数
  * @retval Timer结构体
  */
Timer timerCreate(uint8_t tim, uint16_t period, uint16_t prescaler);

/**
  * @brief  启动定时器（需先使用nvicSetGroup进行优先级分组）
  * @param  timer Timer结构体指针
  * @param  p 定时器溢出中断的回调函数
  * @param  prePriority 定时器抢占优先级
  * @param  subPriority 定时器响应优先级
  */
void timerStart(Timer *timer, void (*p)(), uint8_t prePriority, uint8_t subPriority);

/**
  * @brief  停止定时器
  * @param  timer Timer结构体指针
  */
void timerStop(Timer *timer);

/**
  * @brief  获取定时器溢出中断的回调函数
  * @param  timer Timer结构体指针
  * @retval 定时器溢出中断的回调函数
  */
void* timerGetCallback(Timer *timer);

#endif