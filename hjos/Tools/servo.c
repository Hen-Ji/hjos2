#include "servo.h"

void servoInit(Servo* servo, uint8_t tim, uint8_t channel) {
    servo->pwm = pwmCreate(tim, channel, 20000 - 1, 72 - 1); //创建舵机pwm, 周期20ms
    servo->angle = 0;
}

Servo servoCreate(uint8_t tim, uint8_t channel) {
    Servo servo;
    servoInit(&servo, tim, channel);
    return servo;
}

void servoSetAngle(Servo* servo, int16_t angle) {
    servo->angle = angle;
	pwmSet(&servo->pwm, 500 + 2000 * angle / 180); //在舵机pwm周期为20ms的情况下，每个周期高电平时间为0.5ms时表示舵机转0度，为2.5ms时表示转180度，中间为线性变化
}

void servoStart(Servo* servo) {
	servoSetAngle(servo, servo->angle);
    pwmStart(&servo->pwm);
}

void servoStop(Servo* servo) {
    pwmStop(&servo->pwm);
}