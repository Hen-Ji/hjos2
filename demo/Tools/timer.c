#include "timer.h"

void (*__timerFuncs[3])() = {0};

void timerInit(Timer* timer, uint8_t tim, uint16_t period, uint16_t prescaler) { //初始化
    TIM_TypeDef * tims[] = {TIM2, TIM3, TIM4};
    uint32_t periphs[] = {RCC_APB1Periph_TIM2, RCC_APB1Periph_TIM3, RCC_APB1Periph_TIM4};
    uint8_t channels[] = {TIM2_IRQn, TIM3_IRQn, TIM4_IRQn}; //若这里报错，请确保有 STM32F10X_MD 宏定义

    timer->tim = tims[tim-2];
    timer->id = tim-2;
    timer->period = period;
    timer->prescaler = prescaler;
    timer->channel = channels[tim-2];
    timer->prePriority = 0;
    timer->subPriority = 0;
	
	RCC_APB1PeriphClockCmd(periphs[tim-2], ENABLE); //使能APB1上的计时器
	
	//初始化计时器
	TIM_InternalClockConfig(timer->tim); //使用内部时钟(72MHz)
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //不分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
	TIM_TimeBaseInitStructure.TIM_Period = period; //自动重载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = prescaler; //预分频器的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0; //通用计时器没有这个，给0
	TIM_TimeBaseInit(timer->tim, &TIM_TimeBaseInitStructure);
}

Timer timerCreate(uint8_t tim, uint16_t period, uint16_t prescaler) { //创建
    Timer timer;
    timerInit(&timer, tim, period, prescaler);
    return timer;
}

void timerStart(Timer* timer, void (*p)(), uint8_t prePriority, uint8_t subPriority) { //开始
    __timerFuncs[timer->id] = p;
    timer->prePriority = prePriority;
    timer->subPriority = subPriority;

    NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = timer->channel; //使用此计时器的通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = prePriority; //设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = subPriority; //设置响应优先级
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ITConfig(timer->tim, TIM_IT_Update, ENABLE); //计时器更新(到达重装值后清零时)时中断
	TIM_Cmd(timer->tim, ENABLE); //定时器开始工作
}

void timerStop(Timer* timer) { //停止
	TIM_Cmd(timer->tim, DISABLE); //定时器停止工作
}

void* timerGetCallback(Timer* timer) { //获取回调
    return __timerFuncs[timer->id];
}

//中断函数
void TIM2_IRQHandler(void) {
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) { //判断是否是此计时器更新时产生的中断
		if(__timerFuncs[0]) __timerFuncs[0](); //调用回调
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //清除中断标志位
	}
}
void TIM3_IRQHandler(void) {
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) {
		if(__timerFuncs[1]) __timerFuncs[1](); 
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}
void TIM4_IRQHandler(void) {
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) == SET) {
		if(__timerFuncs[2]) __timerFuncs[2](); 
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	}
}
