#include "global.h"
#include "adc.h"

typedef struct _Joystick {
    Adc adcx, adcy;
}JoyStick;

typedef struct _JoystickStatus {
    float x, y;
}JoystickStatus;

/**
  * @brief  摇杆初始化
  * @param  joy JoyStick结构体指针
  * @param  adcx 摇杆x方向的ADC
  * @param  channelx 摇杆x方向的ADC通道
  * @param  adcy 摇杆y方向的ADC
  * @param  channely 摇杆y方向的ADC通道
  */
void joystickInit(JoyStick* joy, uint8_t adcx, uint8_t channelx, uint8_t adcy, uint8_t channely);

/**
  * @brief  创建摇杆
  * @param  adcx 摇杆x方向的ADC
  * @param  channelx 摇杆x方向的ADC通道
  * @param  adcy 摇杆y方向的ADC
  * @param  channely 摇杆y方向的ADC通道
  * @retval JoyStick结构体
  */
JoyStick joystickCreate(uint8_t adcx, uint8_t channelx, uint8_t adcy, uint8_t channely);

/**
  * @brief  获取摇杆状态
  * @param  joy JoyStick结构体指针
  * @retval JoystickStatus结构体,x,y取值范围均为-1到1,
  */
JoystickStatus joystickGet(JoyStick* joy);