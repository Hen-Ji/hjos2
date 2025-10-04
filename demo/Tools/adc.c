#include "adc.h"

uint8_t adcInited[2] = {0, 0};
uint8_t adcChannels[2] = {0, 0};

void adcInit(Adc* adc, uint8_t num, uint8_t channel) {
    uint8_t ios[] = {
        0, 0, 
        0, 1,
        0, 2,
        0, 3,
        0, 4,
        0, 5,
        0, 6,
        0, 7,
        1, 0,
        1, 1,
        2, 0,
        2, 1,
        2, 2,
        2, 3,
        2, 4,
        2, 5
    };

    ADC_TypeDef* adcs[] = {ADC1, ADC2};
    uint32_t adcPeriphs[] = {RCC_APB2Periph_ADC1, RCC_APB2Periph_ADC2};
    
	//配置引脚
    adc->io = ioCreate(ios[channel*2], ios[channel*2+1], IO_AIN);

	//配置ADC
    num -= 1;

    adc->id = num;
    adc->channel = channel;
    adc->handle = adcs[num];

    if(!adcInited[num]) { //adc还没有初始化
        RCC_APB2PeriphClockCmd(adcPeriphs[num], ENABLE); //开启ADC时钟
        RCC_ADCCLKConfig(RCC_PCLK2_Div6); //设置ADCCLK为6分频（即时钟频率为72/6=12MHz）
	
        ADC_InitTypeDef ADC_InitStructure;
        ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //独立模式
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //右对齐(从寄存器的最右边（最低位）开始存，这样就不用转换数值了)
        ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //不使用外部触发源，使用软件触发
        ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //使用单次转换
        ADC_InitStructure.ADC_ScanConvMode = DISABLE; //使用非扫描模式
        ADC_InitStructure.ADC_NbrOfChannel = 1; //使用前1个序列
        ADC_Init(adcs[num], &ADC_InitStructure);

        ADC_Cmd(adcs[num], ENABLE); //开启ADC

        ADC_ResetCalibration(adcs[num]); //复位校准
        while(ADC_GetResetCalibrationStatus(adcs[num]) == SET); //等待复位校准完成
        ADC_StartCalibration(adcs[num]); //启动校准
        while(ADC_GetCalibrationStatus(adcs[num]) == SET); //等待校准完成

        adcInited[num] = 1; //adc初始化完成
    }
}

Adc adcCreate(uint8_t adcNum, uint8_t channel) {
    Adc adc;
    adcInit(&adc, adcNum, channel);
    return adc;
}

uint16_t adcGet(Adc* adc) {
	ADC_RegularChannelConfig(adc->handle, adc->channel, 1, ADC_SampleTime_13Cycles5); //选择规则组的输入通道，选择通道，放到第1个序列（有16个位置），通道采样时间选择13.5个ADCCLK的时钟周期。
    ADC_SoftwareStartConvCmd(adc->handle, ENABLE); //ADC启动转换
    while (ADC_GetFlagStatus(adc->handle, ADC_FLAG_EOC) == RESET); //获取规则组转换标志位，等待转换完成
    return ADC_GetConversionValue(adc->handle); //获取值(0-4095)。获取完后会自动将标志位置0，不用手动置0
}