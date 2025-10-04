#ifndef __HJOSPORT
#define __HJOSPORT

#include "hjosGlobal.h"
#include "hjos.h"

void hjosCriticalEnter(); //进入临界区
void hjosCriticalExit(); //退出临界区

void hjosSystickHandler(); //SysTick中断函数
__asm void hjosStartFirstTask(); //启动第一个任务
__asm void hjosSvcHandler(); //SVC中断函数
__asm void hjosPendsvHandler(); //PendSV中断函数

#endif