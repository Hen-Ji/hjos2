#include "global.h"
#include "io.h"
#include "pwm.h"

typedef struct _Servo
{
    Pwm pwm;
    int16_t angle;
}Servo;

void servoInit(Servo* servo, uint8_t tim, uint8_t channel);
Servo servoCreate(uint8_t tim, uint8_t channel);
void servoSetAngle(Servo* servo, int16_t angle);
void servoStart(Servo* servo);
void servoStop(Servo* servo);