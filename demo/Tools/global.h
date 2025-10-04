//所有Tools中的库文件都要引入这个头文件，这样移植的时候会比较方便

#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "stm32f10x.h"
#include "stddef.h"
//#define STM32F10X_MD

#define CLOCK_FREQ (72000000) //时钟频率72MHz

#endif
