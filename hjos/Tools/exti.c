#include "exti.h"

void (*__extiFuncs[16])(); //内部回调函数

void extiInit(Exti* exti, uint8_t port, uint8_t pin, uint8_t mode, EXTITrigger_TypeDef trigger) { //初始化
	uint8_t channels[16] = { //若这里报错，请确保有 STM32F10X_MD 宏定义
		EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn, 
		EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, 
		EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn
	};

    ioInit(&(exti->io), port, pin, mode);
    exti->channel = channels[pin];
    exti->trigger = trigger;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //初始化AFIO（复用功能IO）
	
	GPIO_EXTILineConfig(port, pin); //选择引脚通过AFIO连接到EXTI
	
	//初始化EXTI
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = exti->io.pin; //使用这一条
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; //开启
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //使用中断模式
	EXTI_InitStructure.EXTI_Trigger = trigger; //触发模式
	EXTI_Init(&EXTI_InitStructure);
}

Exti extiCreate(uint8_t port, uint8_t pin, uint8_t mode, EXTITrigger_TypeDef trigger) { //创建
    Exti exti;
    extiInit(&exti, port, pin, mode, trigger);
    return exti;
}

void extiStart(Exti* exti, void (*p)(), uint8_t prePriority, uint8_t subPriority) { //启动
	__extiFuncs[exti->io.pinId] = p;
    exti->prePriority = prePriority;
    exti->subPriority = subPriority;

    //初始化NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = exti->channel; //指定中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = prePriority; //设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = subPriority; //设置响应优先级
	NVIC_Init(&NVIC_InitStructure);
}

void extiStop(Exti* exti) { //停止
    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = exti->channel; //指定中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE; //不使能
	NVIC_Init(&NVIC_InitStructure);
}

void* extiGetCallback(Exti* exti) { //获取回调
    return __extiFuncs[exti->io.pinId];
}

//中断函数(在启动文件startup_stm32f10x_md.s中)
void EXTI0_IRQHandler(void) {
	if(__extiFuncs[0]) __extiFuncs[0]();
	EXTI_ClearITPendingBit(EXTI_Line0);
}
void EXTI1_IRQHandler(void) {
	if(__extiFuncs[1]) __extiFuncs[1]();
	EXTI_ClearITPendingBit(EXTI_Line1);
}
void EXTI2_IRQHandler(void) {
	if(__extiFuncs[2]) __extiFuncs[2]();
	EXTI_ClearITPendingBit(EXTI_Line2);
}
void EXTI3_IRQHandler(void) {
	if(__extiFuncs[3]) __extiFuncs[3]();
	EXTI_ClearITPendingBit(EXTI_Line3);
}
void EXTI4_IRQHandler(void) {
	if(__extiFuncs[4]) __extiFuncs[4]();
	EXTI_ClearITPendingBit(EXTI_Line4);
}

void EXTI9_5_IRQHandler(void) {
	if(EXTI_GetITStatus(EXTI_Line5) == SET) { //判断是否是这个中断
		if(__extiFuncs[5]) __extiFuncs[5](); //调用回调
		EXTI_ClearITPendingBit(EXTI_Line5); //清除此中断标志位，这样就不会一直进入中断函数跳不出来
	}
	if(EXTI_GetITStatus(EXTI_Line6) == SET) {
		if(__extiFuncs[6]) __extiFuncs[6]();
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	if(EXTI_GetITStatus(EXTI_Line7) == SET) {
		if(__extiFuncs[7]) __extiFuncs[7]();
		EXTI_ClearITPendingBit(EXTI_Line7);
	}
	if(EXTI_GetITStatus(EXTI_Line8) == SET) {
		if(__extiFuncs[8]) __extiFuncs[8]();
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	if(EXTI_GetITStatus(EXTI_Line9) == SET) {
		if(__extiFuncs[9]) __extiFuncs[9]();
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
}

void EXTI15_10_IRQHandler(void) {
	if(EXTI_GetITStatus(EXTI_Line10) == SET) {
		if(__extiFuncs[10]) __extiFuncs[10]();
		EXTI_ClearITPendingBit(EXTI_Line10);
	}
	if(EXTI_GetITStatus(EXTI_Line11) == SET) {
		if(__extiFuncs[11]) __extiFuncs[11]();
		EXTI_ClearITPendingBit(EXTI_Line11);
	}
	if(EXTI_GetITStatus(EXTI_Line12) == SET) {
		if(__extiFuncs[12]) __extiFuncs[12]();
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
	if(EXTI_GetITStatus(EXTI_Line13) == SET) {
		if(__extiFuncs[13]) __extiFuncs[13]();
		EXTI_ClearITPendingBit(EXTI_Line13);
	}
	if(EXTI_GetITStatus(EXTI_Line14) == SET) {
		if(__extiFuncs[14]) __extiFuncs[14]();
		EXTI_ClearITPendingBit(EXTI_Line14);
	}
	if(EXTI_GetITStatus(EXTI_Line15) == SET) {
		if(__extiFuncs[15]) __extiFuncs[15]();
		EXTI_ClearITPendingBit(EXTI_Line15);
	}
}
