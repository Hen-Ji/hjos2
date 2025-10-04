#ifndef __DELAY_H
#define __DELAY_H

#include "global.h"

/**
  * @brief  纳秒级延时（建议xns > 1000，小了精度不够）
  * @param  xus 延时时长
  */
void delayns(uint32_t xns);

/**
  * @brief  微秒级延时
  * @param  xus 延时时长(0-233015)
  */
void delayus(uint32_t us);

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长(0-4294967295)
  */
void delay(uint32_t ms);
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长(0-4294967295)
  */
void delays(uint32_t s);

#endif
