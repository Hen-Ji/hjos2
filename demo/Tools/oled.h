#ifndef __OLED_H
#define __OLED_H

#include "global.h"
#include "iic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define OLED_ADDRESS 0x3C // 011 1100

typedef struct _Oled {
  IicDevice device;
  uint8_t buffers[2][4][16]; //oledBuffers[0]为正在使用的缓冲，oledBuffers[1]为上次更新时的缓冲
  //使用双缓冲的好处：无论代码写得多拉，在刷新屏幕时仍能够只刷新需要刷新的地方，并且不会重复刷新，使得刷新速度大大提升
  //使用双缓冲的坏处：占用较大的RAM，对内存小的单片机不友好
}Oled;

/**
  * @brief  OLED初始化
  * @param  oled OLED结构体
  * @param  bus OLED需要挂载到的IIC总线
  */
void oledInit(Oled* oled, IicBus* bus);

/**
  * @brief  OLED清屏
  * @param  oled OLED结构体
  */
void oledClear(Oled* oled);

/**
  * @brief  将字符显示在OLED上
  * @param  oled OLED结构体
  * @param  x 需要显示的字符的x轴坐标(0-15)
  * @param  y 需要显示的字符的y轴坐标(0-3)
  * @param  c 需要显示的字符
  */
void oledChar(Oled* oled, uint8_t x, uint8_t y, char c);

/**
  * @brief  字符串存到OLED的缓冲里(使用方式与printf相似)
  * @param  oled OLED结构体
  * @param  x 起始x轴坐标(0-15)
  * @param  y 起始y轴坐标(0-3)
  * @param  format 格式化字符串
  * @param  args... 字符串所带参数
  */
void oledPrint(Oled* oled, uint16_t x, uint16_t y, const char* const format, ...);

/**
  * @brief  将缓冲的内容显示到OLED上
  * @param  oled OLED结构体
  */
void oledUpdate(Oled* oled);

#endif
