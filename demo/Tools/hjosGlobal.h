#ifndef __HJOSGLOBAL_H
#define __HJOSGLOBAL_H

#include "global.h"
#include "heap.h"

#define HJOS_TICKTIM_FREQ (72000000) //节拍定时器计数频率
#define HJOS_TICK_FREQ (1000) //节拍频率
#define HJOS_TICKTIM_RELOAD (HJOS_TICKTIM_FREQ / HJOS_TICK_FREQ) //节拍定时器重装值
#define HJOS_TICKTIM_CNT (HJOS_TICKTIM_RELOAD-SysTick->VAL) //节拍定时器计数值（向上计数）

#define HJOS_IDLE_TASK_STACK_SIZE 64 //空闲任务的任务栈大小，单位：size_t的大小(4Byte)

#define HJOS_PRIORITY_LEVEL 8 //优先级数量(0为最低优先级)

#define HJOS_HEAP_SIZE (8*1024) //堆大小

#define HJOS_TASK_STACK_FILL 0xA5A5A5A5 //任务栈空间预填充值，用于判断任务用掉的任务栈

#define hjosSvcHandler SVC_Handler //重定向
#define hjosPendsvHandler PendSV_Handler
#define hjosSystickHandler SysTick_Handler

#endif