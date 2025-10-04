#ifndef __NVIC_H
#define __NVIC_H

#include "global.h"

/**
  * @brief  中断优先级分组
  * @param  group 组别(0-4)
  */
void nvicSetGroup(uint8_t group);

#endif