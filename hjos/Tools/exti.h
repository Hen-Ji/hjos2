#ifndef __EXTI_H
#define __EXTI_H

#include "global.h"
#include "io.h"

#define EXTI_R EXTI_Trigger_Rising //上升沿触发
#define EXTI_F EXTI_Trigger_Falling //下降沿触发
#define EXTI_RF EXTI_Trigger_Rising_Falling //上升沿下降沿均触发

typedef struct _Exti {
    Io io;
    uint8_t channel;
    EXTITrigger_TypeDef trigger;
    uint8_t prePriority;
    uint8_t subPriority;
}Exti;

/**
  * @brief  外部中断初始化
  * @param  exti Exti结构体指针
  * @param  port io口端口号
  * @param  pin io口引脚号
  * @param  mode io口模式
  * @param  trigger 触发条件
  */
void extiInit(Exti* exti, uint8_t port, uint8_t pin, uint8_t mode, EXTITrigger_TypeDef trigger);

/**
  * @brief  创建Exti结构体并初始化
  * @param  port io口端口号
  * @param  pin io口引脚号
  * @param  mode io口模式
  * @param  trigger 触发条件
  * @retval Exti结构体
  */
Exti extiCreate(uint8_t port, uint8_t pin, uint8_t mode, EXTITrigger_TypeDef trigger);

/**
  * @brief  启动外部中断（需先使用nvicSetGroup进行优先级分组）
  * @param  exti Exti结构体指针
  * @param  p 触发外部中断时的回调函数
  * @param  prePriority 外部中断抢占优先级
  * @param  subPriority 外部中断响应优先级
  */
void extiStart(Exti* exti, void (*p)(), uint8_t prePriority, uint8_t subPriority);

/**
  * @brief  停止外部中断
  * @param  exti Exti结构体指针
  */
void extiStop(Exti* exti);

/**
  * @brief  获取外部中断的回调函数
  * @param  exti Exti结构体指针
  * @retval 外部中断的回调函数
  */
void* extiGetCallback(Exti* exti);

#endif
