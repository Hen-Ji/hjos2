#ifndef __LCDHMINI_H
#define __LCDHMINI_H

#include "global.h"
#include "io.h"
#include "spi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

//#define va_start __crt_va_start
//#define va_arg   __crt_va_arg
//#define va_end   __crt_va_end

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

#define lcdToLegal(a, m) (a < m ? a : m)

#define SPIBUFFER_SIZE (20*40) //不得低于32*16, 否则显示字体可能会溢出
#define COLOR_CONVERT(x) ((uint16_t)(x >> 8) + (uint16_t)(x << 8)) 

void lcdSpiWrite(uint8_t dat); //模拟SPI时序
void lcdWrite8(uint8_t dat);//写入一个字节
void lcdWrite16(uint16_t dat);//写入两个字节
void lcdWriteReg(uint8_t dat);//写入一个指令
void lcdSetAddr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);//设置写入区域（闭区间）

void lcdInit(uint16_t width, uint16_t height, uint8_t isFlip, SpiBus *bus, uint8_t csPort, uint8_t csPin, uint8_t resPort, uint8_t resPin, uint8_t dcPort, uint8_t dcPin, uint8_t blkPort, uint8_t blkPin);//LCD初始化

void lcdFill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);//指定区域填充颜色
void lcdPoint(uint16_t x,uint16_t y,uint16_t color);//在指定位置画一个点
void lcdLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);//在指定位置画一条线
void lcdRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);//在指定位置画一个矩形
void lcdCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color);//在指定位置画一个圆

void lcdChar(uint16_t x, uint16_t y, uint16_t height, uint16_t color, uint16_t back, char ch); //显示字符
void lcdPrint(uint16_t x, uint16_t y, uint16_t height, uint16_t color, uint16_t back, const char* const format, ...);//显示字符串

void lcdPicture(uint16_t x,uint16_t y,uint16_t length,uint16_t width,const uint16_t pic[]);//显示图片
#endif




