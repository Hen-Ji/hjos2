#include "lcdhmini.h"
#include "lcdfont.h"
#include "spi.h"
#include "delay.h"

uint16_t lcdWidth = 160, lcdHeight = 80;
uint8_t lcdIsFilp = 0;
Io lcdRes, lcdDc, lcdBlk;
SpiDevice lcdDevice;

uint16_t spiBuffer[SPIBUFFER_SIZE] = {0}; //spi传输缓存（由于每个数据的低位存在内存的低位，即如0xABCD, spiBuffer指向的地址在那个字节存的单个字节的数据其实是0xCD，而不是0xAB, 所以传输的时候颜色需要将高8位和低8位互换）

void lcdSpiWrite(uint8_t dat) {	//模拟SPI时序
	// lcdSetCs(0);
	// for(uint8_t i=0;i<8;i++) {		
	// 	lcdSetScl(0);
	// 	lcdSetSda(dat & (0x80 >> i));
	// 	lcdSetScl(1);
	// }	
  	// lcdSetCs(1);	

	//这样写刷屏速度快了2/3, 最终刷一次整片屏幕所需时间约为30ms
}

void lcdWrite8(uint8_t dat) {//写入一个字节
	spiTransmit(&lcdDevice, &dat, 1);	
}

void lcdWrite16(uint16_t dat) {//写入两个字节
	uint8_t arr[2] = {(uint8_t)(dat>>8), (uint8_t)dat};
	spiTransmit(&lcdDevice, arr, 2);	
}

void lcdWriteReg(uint8_t dat) {//写入一个指令
	ioSet(&lcdDc, 0);//写命令
	spiTransmit(&lcdDevice, &dat, 1);	
	ioSet(&lcdDc, 1);//写数据
}

void lcdSetAddr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {//设置写入区域（闭区间）
	lcdWriteReg(0x2a);//列地址设置
	lcdWrite16(x1+1);
	lcdWrite16(x2+1);
	lcdWriteReg(0x2b);//行地址设置
	lcdWrite16(y1+26);
	lcdWrite16(y2+26);
	lcdWriteReg(0x2c);//储存器写
}

void lcdInit(uint16_t width, uint16_t height, uint8_t isFlip, SpiBus *bus, uint8_t csPort, uint8_t csPin, uint8_t resPort, uint8_t resPin, uint8_t dcPort, uint8_t dcPin, uint8_t blkPort, uint8_t blkPin) {//LCD初始化
	lcdWidth = width;
	lcdHeight = height;
	lcdIsFilp = isFlip;
	lcdDevice = spiCreateDevice(bus, csPort, csPin);
	lcdRes = ioCreate(resPort, resPin, IO_PP);
	lcdDc = ioCreate(dcPort, dcPin, IO_PP);
	lcdBlk = ioCreate(blkPort, blkPin, IO_PP);

	ioSet(&lcdRes, 0);//复位
	delay(100);
	ioSet(&lcdRes, 1);
	delay(100);
	
	ioSet(&lcdBlk, 1);//打开背光
  	delay(100);
	
	lcdWriteReg(0x11);     //Sleep out
	delay(120);                //Delay 120ms
	lcdWriteReg(0xB1);     //Normal mode
	lcdWrite8(0x05);   
	lcdWrite8(0x3C);   
	lcdWrite8(0x3C);   
	lcdWriteReg(0xB2);     //Idle mode
	lcdWrite8(0x05);   
	lcdWrite8(0x3C);   
	lcdWrite8(0x3C);   
	lcdWriteReg(0xB3);     //Partial mode
	lcdWrite8(0x05);   
	lcdWrite8(0x3C);   
	lcdWrite8(0x3C);   
	lcdWrite8(0x05);   
	lcdWrite8(0x3C);   
	lcdWrite8(0x3C);   
	lcdWriteReg(0xB4);     //Dot inversion
	lcdWrite8(0x03);   
	lcdWriteReg(0xC0);     //AVDD GVDD
	lcdWrite8(0xAB);   
	lcdWrite8(0x0B);   
	lcdWrite8(0x04);   
	lcdWriteReg(0xC1);     //VGH VGL
	lcdWrite8(0xC5);   //C0
	lcdWriteReg(0xC2);     //Normal Mode
	lcdWrite8(0x0D);   
	lcdWrite8(0x00);   
	lcdWriteReg(0xC3);     //Idle
	lcdWrite8(0x8D);   
	lcdWrite8(0x6A);   
	lcdWriteReg(0xC4);     //Partial+Full
	lcdWrite8(0x8D);   
	lcdWrite8(0xEE);   
	lcdWriteReg(0xC5);     //VCOM
	lcdWrite8(0x0F);   
	lcdWriteReg(0xE0);     //positive gamma
	lcdWrite8(0x07);   
	lcdWrite8(0x0E);   
	lcdWrite8(0x08);   
	lcdWrite8(0x07);   
	lcdWrite8(0x10);   
	lcdWrite8(0x07);   
	lcdWrite8(0x02);   
	lcdWrite8(0x07);   
	lcdWrite8(0x09);   
	lcdWrite8(0x0F);   
	lcdWrite8(0x25);   
	lcdWrite8(0x36);   
	lcdWrite8(0x00);   
	lcdWrite8(0x08);   
	lcdWrite8(0x04);   
	lcdWrite8(0x10);   
	lcdWriteReg(0xE1);     //negative gamma
	lcdWrite8(0x0A);   
	lcdWrite8(0x0D);   
	lcdWrite8(0x08);   
	lcdWrite8(0x07);   
	lcdWrite8(0x0F);   
	lcdWrite8(0x07);   
	lcdWrite8(0x02);   
	lcdWrite8(0x07);   
	lcdWrite8(0x09);   
	lcdWrite8(0x0F);   
	lcdWrite8(0x25);   
	lcdWrite8(0x35);   
	lcdWrite8(0x00);   
	lcdWrite8(0x09);   
	lcdWrite8(0x04);   
	lcdWrite8(0x10);
		 
	lcdWriteReg(0xFC);    
	lcdWrite8(0x80);  
		
	lcdWriteReg(0x3A);     
	lcdWrite8(0x05);   
	lcdWriteReg(0x36);
	if(isFlip) lcdWrite8(0x78);
	else lcdWrite8(0xA8);   
	lcdWriteReg(0x21);     //Display inversion
	lcdWriteReg(0x29);     //Display on
	lcdWriteReg(0x2A);     //Set Column Address
	lcdWrite8(0x00);   
	lcdWrite8(0x1A);  //26  
	lcdWrite8(0x00);   
	lcdWrite8(0x69);   //105 
	lcdWriteReg(0x2B);     //Set Page Address
	lcdWrite8(0x00);   
	lcdWrite8(0x01);    //1
	lcdWrite8(0x00);   
	lcdWrite8(0xA0);    //160
	lcdWriteReg(0x2C); 
}

void lcdPoint(uint16_t x,uint16_t y,uint16_t color) { //画点
	lcdSetAddr(x, y, x, y);
	lcdWrite16(color);
} 

void lcdFill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color) { //填充区域 
	color = COLOR_CONVERT(color); //到缓存的颜色高8位和低8位需要互换，否则颜色输出不正确

	uint32_t pixels = (xend-xsta+1) * (yend-ysta+1);
	uint32_t i = 0;
	while(i < pixels && i < SPIBUFFER_SIZE) {
		spiBuffer[i] = color;
		i++;
	}

	lcdSetAddr(xsta, ysta, xend-1, yend-1);
	while(pixels >= SPIBUFFER_SIZE) {
		spiTransmit(&lcdDevice, (uint8_t*)spiBuffer, SPIBUFFER_SIZE*2); //buffer只有这么大，需要多次输出
		pixels -= SPIBUFFER_SIZE;
	}
	if(pixels > 0) {
		spiTransmit(&lcdDevice, (uint8_t*)spiBuffer, pixels*2);
	}

}

void lcdLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color) { //画线
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向 
	else if (delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		lcdPoint(uRow,uCol,color);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

void lcdRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color) { //画矩形
	lcdLine(x1,y1,x2,y1,color);
	lcdLine(x1,y1,x1,y2,color);
	lcdLine(x1,y2,x2,y2,color);
	lcdLine(x2,y1,x2,y2,color);
}

void lcdCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color) { //画圆
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		lcdPoint(x0-b,y0-a,color);             //3           
		lcdPoint(x0+b,y0-a,color);             //0           
		lcdPoint(x0-a,y0+b,color);             //1                
		lcdPoint(x0-a,y0-b,color);             //2             
		lcdPoint(x0+b,y0+a,color);             //4               
		lcdPoint(x0+a,y0-b,color);             //5
		lcdPoint(x0+a,y0+b,color);             //6 
		lcdPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//判断要画的点是否过远
		{
			b--;
		}
	}
}

void lcdChar(uint16_t x, uint16_t y, uint16_t height, uint16_t color, uint16_t back, char ch) { //画字符
	color = COLOR_CONVERT(color);
	back = COLOR_CONVERT(back);

	ch -= ' '; //减去空格，得到字符在数组中对应的下标
	uint16_t width = height / 2;
	uint16_t status = 0, idx = 0;
	uint32_t bufferIdx = 0;
	
	int32_t h = lcdToLegal(height, (int32_t)lcdHeight - y); //使其不溢出
	int32_t w = lcdToLegal(width, (int32_t)lcdWidth - x);

	for(uint8_t i = 0; i < h; i++) {
		for(uint8_t j = 0; j < w; j++) {
			if(height == 12) {
				idx = i*8 + j;
				status = (uint8_t)ascii_1206[(uint8_t)ch][idx / 8] & (1 << (idx % 8));
			}
			else if (height == 16) {
				idx = i*8 + j;
				status = (uint8_t)ascii_1608[(uint8_t)ch][idx / 8] & (1 << (idx % 8));
			}
			// else if (height == 24) {
			// 	idx = i*16 + j;
			// 	status = (uint8_t)ascii_2412[(uint8_t)ch][idx / 8] & (1 << (idx % 8));
			// }
			// else if (height == 32) {
			// 	idx = i*16 + j;
			// 	status = (uint8_t)ascii_3216[(uint8_t)ch][idx / 8] & (1 << (idx % 8));
			// }

			//lcdWrite16(status ? color : back);
			spiBuffer[bufferIdx++] = status ? color : back;
		}
	}  	

	lcdSetAddr(x, y, x+w-1, y+h-1);
	if(bufferIdx) spiTransmit(&lcdDevice, (uint8_t*)spiBuffer, bufferIdx*2);
}

void lcdPrint(uint16_t x, uint16_t y, uint16_t height, uint16_t color, uint16_t back, const char* const format, ...) { //打印格式化字符串 
	//在makefile中的LDFLAGS删去 -specs=nano.specs 就能输出浮点数了，但是相应的烧录文件会变大(FLASH变大大概20K)        
	va_list args;
	va_start(args, format);

	char str[64] = {0};
	uint16_t i = 0;
	vsprintf(str, format, args); //格式化字符串转字符串
	while(i < 64 && str[i]!='\0')
	{       
		lcdChar(x, y, height, color, back, str[i]);
		x+=height/2;
		i++;
	}  

	va_end(args);
}

void lcdPicture(uint16_t x,uint16_t y,uint16_t height,uint16_t width,const uint16_t pic[]) { //画画	
	int32_t h = lcdToLegal(height, (int32_t)lcdHeight - y); //使其不溢出
	int32_t w = lcdToLegal(width, (int32_t)lcdWidth - x);
	uint32_t bufferIdx = 0;

	lcdSetAddr(x, y, x+w-1, y+h-1);

	for(uint16_t i=0;i<h*w;i++) {
		spiBuffer[bufferIdx++] = COLOR_CONVERT(pic[i]);

		if(bufferIdx == SPIBUFFER_SIZE) {
			spiTransmit(&lcdDevice, (uint8_t*)spiBuffer, SPIBUFFER_SIZE*2); //可能有很多，分几次传
			bufferIdx = 0;
		}
	}
	if(bufferIdx) 
			spiTransmit(&lcdDevice, (uint8_t*)spiBuffer, bufferIdx*2);
}
