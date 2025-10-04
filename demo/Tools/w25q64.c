#include "w25q64.h"

SpiDevice w25q64Device;

void w25q64Init(SpiBus *bus, uint8_t csPort, uint8_t csPin) {
    w25q64Device = spiCreateDevice(bus, csPort, csPin);
}

void w25q64WriteEnable() {
    uint8_t data = 0x06;
    spiTransmit(&w25q64Device, &data, 1); //写使能
}

uint8_t w25q64IsBusy() {
    w25q64WriteEnable();
    uint8_t data = 0x05, res;
    spiReceive(&w25q64Device, &data, 1, &res, 1); //读取状态
	if((res & 0x01) == 0x01) return 1; //busy位为最低位
	else return 0;
}

void w25q64Erase(uint32_t addr) { //擦除
	while(w25q64IsBusy() == 1); //等待存储器不忙
	w25q64WriteEnable(); //写使能

    uint8_t datas[] = {
        0x20, //擦除扇区(4KB)指令
        addr >> 16, addr >> 8, addr  //传地址
    };
	
    spiTransmit(&w25q64Device, datas, 4);
}

void w25q64Write(uint32_t addr, uint8_t* arr, uint32_t size) { //写数据(页编程)(可以最多写入256个数据且不能跨页?)  write前要先erase对应的块
	while(w25q64IsBusy() == 1); //等待存储器不忙
	w25q64WriteEnable(); //写使能
	
    uint8_t datas[] = {
        0x02, //页编程指令(即写入)
        addr >> 16, addr >> 8, addr  //传地址
    };
    spiBusStart(w25q64Device.bus, &w25q64Device.cs);
    spiBusSwap(w25q64Device.bus, datas, 0, 4);
    spiBusSwap(w25q64Device.bus, arr, 0, size);
    spiBusStop(w25q64Device.bus, &w25q64Device.cs);
}

void w25q64Read(uint32_t addr, uint8_t* arr, uint32_t size) { //读数据(可以读取超过256字节且可以跨页读取)
	while(w25q64IsBusy() == 1); //等待存储器不忙
	w25q64WriteEnable(); //写使能
	
    uint8_t datas[] = {
        0x03, //读数据指令
        addr >> 16, addr >> 8, addr  //传地址
    };
	spiReceive(&w25q64Device, datas, 4, arr, size);
}

void w25q64ReadID(uint8_t *mid, uint16_t *did) { //读取设备相关信息
    uint8_t data = 0x9F; //发读取ID的指令
    uint8_t res[3];
	spiReceive(&w25q64Device, &data, 1, res, 3);
    *mid = res[0]; //厂商ID
    *did = res[1] << 8 | res[2]; //存储器类型, 容量
}
