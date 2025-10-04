#include "oled.h"
#include "io.h"
#include "oledFont.h"

void oledSendCommand(Oled* oled, uint8_t data) { //发送命令
	uint8_t d[] = {0, data};
	iicTransmit(&oled->device, d, 2);
}

void oledSendData(Oled* oled, uint8_t data) { //发送数据
	uint8_t d[] = {0x40, data};
	iicTransmit(&oled->device, d, 2);
}

void oledSendDatas(Oled* oled, uint8_t *datas, uint8_t size) { //发送多个数据
	uint8_t d[] = {0x40};
	iicBusStart(oled->device.bus);
	iicBusTransmitAddr(oled->device.bus, OLED_ADDRESS, IIC_WRITE);
	iicBusTransmit(oled->device.bus, d, 1);
	iicBusTransmit(oled->device.bus, datas, size);
	iicBusStop(oled->device.bus);
}

void oledSetCursor(Oled* oled, uint8_t x, uint8_t y) { //设置光标
	oledSendCommand(oled, 0xB0 | y);					//设置Y位置
	oledSendCommand(oled, 0x10 | ((x & 0xF0) >> 4));	//设置X位置高4位
	oledSendCommand(oled, 0x00 | (x & 0x0F));			//设置X位置低4位
}

void oledInit(Oled* oled, IicBus* bus) { //初始化
	oled->device = iicCreateDevice(bus, OLED_ADDRESS);

	//初始化缓冲
	memset(oled->buffers, ' ', 4 * 16);

	uint32_t i, j;
	for (i = 0; i < 1000; i++) for (j = 0; j < 1000; j++);			//上电延时
	
	//初始化代码，应该可以从数据手册上查到demo
	oledSendCommand(oled, 0xAE);	//关闭显示
	oledSendCommand(oled, 0xD5);	//设置显示时钟分频比/振荡器频率
	oledSendCommand(oled, 0x80);
	oledSendCommand(oled, 0xA8);	//设置多路复用率
	oledSendCommand(oled, 0x3F);
	oledSendCommand(oled, 0xD3);	//设置显示偏移
	oledSendCommand(oled, 0x00);
	oledSendCommand(oled, 0x40);	//设置显示开始行
	oledSendCommand(oled, 0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	oledSendCommand(oled, 0xC8);	//设置上下方向，0xC8正常 0xC0上下反置
	oledSendCommand(oled, 0xDA);	//设置COM引脚硬件配置
	oledSendCommand(oled, 0x12);
	oledSendCommand(oled, 0x81);	//设置对比度控制
	oledSendCommand(oled, 0xCF);
	oledSendCommand(oled, 0xD9);	//设置预充电周期
	oledSendCommand(oled, 0xF1);
	oledSendCommand(oled, 0xDB);	//设置VCOMH取消选择级别
	oledSendCommand(oled, 0x30);
	oledSendCommand(oled, 0xA4);	//设置整个显示打开/关闭
	oledSendCommand(oled, 0xA6);	//设置正常/倒转显示
	oledSendCommand(oled, 0x8D);	//设置充电泵
	oledSendCommand(oled, 0x14);
	oledSendCommand(oled, 0xAF);	//开启显示

	oledClear(oled); //先把缓冲0填满空格
	oledUpdate(oled); //由于缓冲1的内容全是0，与空格不一样，因此这一次更新相当于把整个OLED都填满空格，不需要另写清屏代码了
}

void oledClear(Oled* oled) { //清屏
	memset(oled->buffers, ' ', 4 * 16); //将缓冲里面的内容全部用空格替代
}

void oledChar(Oled* oled, uint8_t x, uint8_t y, char c) { //显示字符
	oledSetCursor(oled, x * 8, y * 2); //设置光标位置在上半部分
	oledSendDatas(oled, (uint8_t*)oledFont_8x16[c - ' '], 8); //显示上半部分内容

	oledSetCursor(oled, x * 8, y * 2 + 1); //设置光标位置在下半部分
	oledSendDatas(oled, (uint8_t*)&(oledFont_8x16[c - ' '][8]), 8); //显示下半部分内容
}

void oledPrint(Oled* oled, uint16_t x, uint16_t y, const char* const format, ...) { //输出
	va_list args;
	va_start(args, format);

	char str[64] = {0};
	uint16_t i = 0, begin = x;
	vsprintf(str, format, args); //格式化字符串转字符串
	while(str[i]!='\0') {
		if(str[i] == '\n') { //换行
			y++;
			x = begin;
			i++;
			continue; //换下一行
		}
		if(x >= 0 && x < 16 && y >= 0 && y < 4) oled->buffers[0][y][x] = str[i]; //存到缓冲中
		x++; //下一个
		i++;
	}  

	va_end(args);
}

void oledUpdate(Oled* oled) { //更新
	for(uint8_t y = 0; y < 4; y++) {
		for(uint8_t x = 0; x < 16; x++) {
			if(oled->buffers[0][y][x] != oled->buffers[1][y][x]) { //此字符不同，更新一下
				oledChar(oled, x, y, oled->buffers[0][y][x]);
			}
			oled->buffers[1][y][x] = oled->buffers[0][y][x]; //保存缓冲
		}
	}
}