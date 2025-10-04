/* ADC引脚定义
               ADC1   ADC2
    channel0   PA0    PA0
    channel1   PA1    PA1
    channel2   PA2    PA2
    channel3   PA3    PA3
    channel4   PA4    PA4
    channel5   PA5    PA5
    channel6   PA6    PA6
    channel7   PA7    PA7
    channel8   PB0    PB0
    channel9   PB1    PB1
    channel10  PC0    PC0
    channel11  PC1    PC1
    channel12  PC2    PC2
    channel13  PC3    PC3
    channel14  PC4    PC4
    channel15  PC5    PC5
*/

#include "global.h"
#include "io.h"

typedef struct _Adc {
    Io io;
    ADC_TypeDef * handle;
    uint8_t id;
    uint8_t channel;
}Adc;

/**
  * @brief  ADC初始化
  * @param  adc Adc结构体指针
  * @param  num 使用的ADC(1-3)
  * @param  channel ADC通道
  */
void adcInit(Adc* adc, uint8_t num, uint8_t channel);

/**
  * @brief  创建ADC
  * @param  num 使用的ADC(1-3)
  * @param  channel ADC通道
  * @retval Adc结构体
  */
Adc adcCreate(uint8_t adcNum, uint8_t channel);

/**
  * @brief  获取ADC的值
  * @param  adc Adc结构体指针
  * @retval ADC的值(0-4095)
  */
uint16_t adcGet(Adc* adc);