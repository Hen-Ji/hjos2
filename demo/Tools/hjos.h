//纯系统(仅计算hjos.o和hjosPort.o)仅需 2122Byte ROM + 180Byte RAM
#ifndef __HJOS_H
#define __HJOS_H

#include "hjosGlobal.h"
#include "hjosPort.h"

#define HJOS_TASK_BUSY 0x01 //任务是否在运行掩码
#define HJOS_TASK_STOP 0x02 //任务是否停止掩码
#define HJOS_TASK_SUSPEND 0x04 //任务是否挂起掩码
#define HJOS_TASK_ERR 0x80 //任务是否出现错误掩码

typedef struct _HjosList { //链表
    void* val;
    struct _HjosList *prev;
    struct _HjosList *next;
}HjosList;

typedef struct _HjosTask { //任务
  uint8_t state; //状态
  uint8_t priority; //优先级
  char* name; //名称
  void* arg; //参数
  void (*callback)(void*); //任务回调
  size_t stackSize; //任务栈大小
  uint64_t delayToTick; //延时到某tick继续执行
  HjosList* node; //任务的任务控制块在任务列表中的节点
}HjosTask;

typedef struct _HjosTcb { //任务控制块
  volatile size_t* topOfStack; //任务栈的栈顶指针
  HjosTask* task; //任务
  size_t* stack; //任务栈地址
}HjosTcb;

typedef struct _HjosQueue { //消息队列
  size_t maxSize;
  size_t itemSize;
  size_t begin, size;
  uint8_t* buffer;
}HjosQueue;

/**
  * @brief  系统初始化，需要在其他hjos相关的函数调用之前调用
  */
void hjosInit();

/**
  * @brief  启动系统，需要在系统依赖的Tick时钟启动后调用
  */
void hjosStart();

/**
  * @brief  启动系统调度器，一般情况不需要主动调用
  */
void hjosStartScheduler();

/**
  * @brief  切换任务上下文，一般情况不需要主动调用
  */
void hjosTaskSwitchContext();


/**
  * @brief  创建任务
  * @param  name 任务名称
  * @param  cb 任务函数回调
  * @param  arg 任务参数
  * @param  stackSize 任务栈大小，单位: size_t的大小(4Byte)
  * @param  priority 优先级，值越大，优先级越高
  * @retval 任务控制块结构体，包含任务结构体
  */
HjosTcb* hjosTaskCreate(char* name, void (*cb)(void*), void* arg, uint32_t stackSize, uint8_t priority);

/**
  * @brief  删除任务，在任务删除之后需要留一定时间给空闲任务，否则任务资源无法释放
  * @param  tcb 任务控制块结构体，为0则删除当前任务
  */
void hjosTaskDelete(HjosTcb* tcb);

/**
  * @brief  主动切换任务
  */
void hjosTaskSwitch();

/**
  * @brief  任务延迟一些tick再继续执行，期间会执行其他任务
  * @param  tick 任务延时，单位为tick
  */
void hjosTaskDelay(uint64_t tick);

/**
  * @brief  任务延迟至某tick再继续执行，期间会执行其他任务
  * @param  tick 任务延时，单位为tick
  */
void hjosTaskDelayTo(uint64_t tick);

/**
  * @brief  暂停执行此任务，用于切换至下一个任务，一般情况下应使用hjosTaskSwitch()
  */
void hjosTaskYield();

/**
  * @brief  挂起任务
  * @param  tcb 任务控制块结构体，为0则挂起当前任务
  */
void hjosTaskSuspend(HjosTcb* tcb);

/**
  * @brief  恢复任务
  * @param  tcb 任务控制块结构体，为0则恢复当前任务（但实际上，被挂起的任务不能自己恢复）
  */
void hjosTaskResume(HjosTcb* tcb);


/**
  * @brief  创建消息队列
  * @param  maxSize 队列的最大长度
  * @param  itemSize 消息的大小
  * @retval 消息队列结构体
  */
HjosQueue* hjosQueueCreate(size_t maxSize, size_t itemSize);

/**
  * @brief  发送消息
  * @param  queue 消息队列结构体
  * @param  item 要发送的消息
  * @retval 0为消息队列已满，1为发送成功
  */
uint8_t hjosQueueSend(HjosQueue* queue, void* item);

/**
  * @brief  接收消息
  * @param  queue 消息队列结构体
  * @param  item 接收到的消息存在这里
  * @param  timeout 最大等待时间，为0则不等待
  * @retval 0为消息队列为空，1为接收成功
  */
uint8_t hjosQueueReceive(HjosQueue* queue, void* item, uint64_t timeout);

/**
  * @brief  在节点后插入新的节点
  * @param  node 链表节点结构体
  * @param  node 链表值
  */
void hjosListInsert(HjosList* node, void* val);

/**
  * @brief  删除链表节点
  * @param  node 链表节点结构体
  */
void hjosListErase(HjosList* node);


/**
  * @brief  获取任务运行过程中，任务栈中空闲的堆栈大小的历史最小值，单位：size_t的大小(4Byte)。注：仅推荐调试时用，并且在某些情况下可能会不准确
  * @param  tcb 任务控制块结构体
  */
size_t hjosGetTaskStackIdle(HjosTcb* tcb);

/**
  * @brief  获取当前tick
  */
uint64_t hjosGetCurrentTick();

/**
  * @brief  获取上一tick的CPU占用率
  */
uint16_t hjosGetUsage();

/**
  * @brief  获取CPU平均占用率
  */
uint16_t hjosGetUsageAvg();

/**
  * @brief  获取当前任务总数
  */
size_t hjosGetTaskSize();


/**
  * @brief  设置系统进入空闲状态前的函数回调，该函数在空闲任务中被调用
  * @param  p 回调函数
  */
void hjosSetIdleCallback(void (*p)());

/**
  * @brief  设置系统进入忙碌状态前的函数回调，该函数在PendSV中断中被调用
  * @param  p 回调函数
  */
void hjosSetBusyCallback(void (*p)());

/**
  * @brief  设置任务切换前的函数回调，该函数在PendSV中断中被调用
  * @param  p 回调函数
  */
void hjosSetSwitchCallback(void (*p)());

/**
  * @brief  设置任务切换完成时的函数回调，该函数在PendSV中断中被调用
  * @param  p 回调函数
  */
void hjosSetSwitchedCallback(void (*p)());

#endif