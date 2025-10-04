#include "pwm.h"


void pwmInit(Pwm *pwm, uint8_t tim, uint8_t channel, uint16_t period, uint16_t prescaler) { //初始化
    pwm->channel = channel;
    pwm->cmp = 0;

    //初始化定时器和io
    uint8_t pins[3][4] = { //每个定时器和通道的组合对应的io口都不一样，需要列表查找
        0, 1, 2, 3,
        6, 7, 0, 1,
        6, 7, 8, 9
    };
    uint8_t ports[3][4] = {
        0, 0, 0, 0,
        0, 0, 1, 1,
        1, 1, 1, 1
    };
    timerInit(&(pwm->timer), tim, period, prescaler); //初始化相应定时器
    ioInit(&(pwm->io), ports[tim-2][channel-1], pins[tim-2][channel-1], IO_AFPP); //io口使用复用推挽输出

    //初始化模式
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure); //给结构体赋初值
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //使用PWM模式1(计时器在使用向上计数模式时，CNT(计时器的计数) < CCR(比较寄存器) 时，REF置有效电平；CNT >= CCR 时，REF置无效电平)
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //有效电平为高电平
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //输出使能
    TIM_OCInitStructure.TIM_Pulse = 0; //设置CCR的值, 先设0
    if(channel == 1) TIM_OC1Init(pwm->timer.tim, &TIM_OCInitStructure); //选通道
    else if(channel == 2) TIM_OC2Init(pwm->timer.tim, &TIM_OCInitStructure);
    else if(channel == 3) TIM_OC3Init(pwm->timer.tim, &TIM_OCInitStructure);
    else if(channel == 4) TIM_OC4Init(pwm->timer.tim, &TIM_OCInitStructure);
}

Pwm pwmCreate(uint8_t tim, uint8_t channel, uint16_t period, uint16_t prescaler) { //创建
    Pwm pwm;
    pwmInit(&pwm, tim, channel, period, prescaler);
    return pwm;
}

void pwmStart(Pwm *pwm) { //开始生成pwm
    TIM_Cmd(pwm->timer.tim, ENABLE); //定时器开始工作
    TIM_CtrlPWMOutputs(pwm->timer.tim, ENABLE); //使能外设输出
}

void pwmSet(Pwm *pwm, uint16_t cmp) { //重新设置CCR的值
    pwm->cmp = cmp;
	if(pwm->channel == 1) TIM_SetCompare1(pwm->timer.tim, cmp); //根据通道选用不同的函数
	else if(pwm->channel == 2) TIM_SetCompare2(pwm->timer.tim, cmp);
	else if(pwm->channel == 3) TIM_SetCompare3(pwm->timer.tim, cmp);
	else if(pwm->channel == 4) TIM_SetCompare4(pwm->timer.tim, cmp);
}

void pwmStop(Pwm *pwm) { //停止
    TIM_Cmd(pwm->timer.tim, DISABLE); //定时器停止工作
    TIM_CtrlPWMOutputs(pwm->timer.tim, DISABLE); //停止外设输出
}