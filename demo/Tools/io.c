#include "io.h"

void ioInit(Io* io, uint8_t port, uint8_t pin, uint8_t mode) {
    uint32_t ioPeriphClock[] = {RCC_APB2Periph_GPIOA, RCC_APB2Periph_GPIOB, RCC_APB2Periph_GPIOC, RCC_APB2Periph_GPIOD};
    GPIO_TypeDef * ioPort[] = {GPIOA, GPIOB, GPIOC, GPIOD};
    const uint16_t ioPin[] = {
        GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_3, 
        GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7, 
        GPIO_Pin_8, GPIO_Pin_9, GPIO_Pin_10, GPIO_Pin_11, 
        GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15
    };
    const GPIOMode_TypeDef ioMode[] = {
        GPIO_Mode_AIN,
        GPIO_Mode_IN_FLOATING,
        GPIO_Mode_IPD,
        GPIO_Mode_IPU,
        GPIO_Mode_Out_OD,
        GPIO_Mode_Out_PP,
        GPIO_Mode_AF_OD,
        GPIO_Mode_AF_PP
    };

	io->port = ioPort[port];
	io->mode = ioMode[mode];
	io->pin = ioPin[pin];
    io->pinId = pin;
    io->portId = port;

    RCC_APB2PeriphClockCmd(ioPeriphClock[port], ENABLE); //使能外设
	
	GPIO_InitTypeDef GPIO_InitStructure; //定义初始化结构体
	GPIO_InitStructure.GPIO_Mode = io->mode; //使用哪种输出
	GPIO_InitStructure.GPIO_Pin = io->pin; //配置引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //选择频率
	GPIO_Init(io->port, &GPIO_InitStructure); //初始化
}

Io ioCreate(uint8_t port, uint8_t pin, uint8_t mode) {
    Io io;
    ioInit(&io, port, pin, mode);
    return io;
}

void ioSet(Io* io, uint8_t val) {
    GPIO_WriteBit(io->port, io->pin, val);
}

uint8_t ioGet(Io* io) {
    return GPIO_ReadInputDataBit(io->port, io->pin);
}
