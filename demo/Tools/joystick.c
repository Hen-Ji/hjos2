#include "joystick.h"

void joystickInit(JoyStick* joy, uint8_t adcx, uint8_t channelx, uint8_t adcy, uint8_t channely) {
    joy->adcx = adcCreate(adcx, channelx);
    joy->adcy = adcCreate(adcy, channely);
}

JoyStick joystickCreate(uint8_t adcx, uint8_t channelx, uint8_t adcy, uint8_t channely) {
    JoyStick joy;
    joystickInit(&joy, adcx, channelx, adcy, channely);
    return joy;
}

JoystickStatus joystickGet(JoyStick* joy) {
    JoystickStatus status;
    status.x = adcGet(&joy->adcx) / 4095.0 * 2 - 1;
    status.y = adcGet(&joy->adcy) / 4095.0 * 2 - 1;
    return status;
}