#include "uart.h"

void (*__uartReceiveCb[3])(uint8_t) = {0};
void (*__uartStringReceiveCb)(char*, uint32_t) = 0;


void uartDefaultCb(uint8_t byte) { //内置接收函数回调
    static char str[2][128] = {0}; //双缓冲
    static uint32_t idx = 0;
    static uint8_t now = 0;
    str[now][idx++] = byte;
    if(byte == 0 || idx >= 127) { //结束或溢出时处理数据
        if(__uartStringReceiveCb) __uartStringReceiveCb(str[now], idx); //处理
        now = (now + 1) % 2; //切换缓冲
        idx = 0;
    }
}

void uartInitHardware(Uart* uart, uint8_t num, uint32_t baud) { //初始化
    uint32_t periphs[] = {RCC_APB2Periph_USART1, RCC_APB1Periph_USART2, RCC_APB1Periph_USART3};
    USART_TypeDef* handles[] = {USART1, USART2, USART3};
    uint8_t ports[] = {0, 0, 1};
    uint8_t txs[] = {9, 2, 10};
    uint8_t rxs[] = {10, 3, 11};
    num--;

    uart->tx = ioCreate(ports[num], txs[num], IO_AFPP); //使用复用推挽输出
    uart->rx = ioCreate(ports[num], rxs[num], IO_PU); //使用上拉输入
    uart->handle = handles[num];
    uart->baud = (baud == 0 ? 115200 : baud); //波特率，为0则默认115200
    uart->parity = USART_Parity_No; //无校验
    uart->stopBits = USART_StopBits_1; //1位停止位
    uart->dataBits = USART_WordLength_8b; //一次传8bit数据
    __uartReceiveCb[num] = 0;
    uart->id = num; //id为uart的序号减1

	RCC_APB2PeriphClockCmd(periphs[num], ENABLE); //开启时钟
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = uart->baud;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //不使用流控制
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; //使用TX和RX
	USART_InitStructure.USART_Parity = uart->parity;
	USART_InitStructure.USART_StopBits = uart->stopBits;
	USART_InitStructure.USART_WordLength = uart->dataBits;
	USART_Init(uart->handle, &USART_InitStructure);
	
	USART_Cmd(uart->handle, ENABLE); //使能
}

Uart uartCreateHardware(uint8_t num, uint32_t baud) {
    Uart uart;
    uartInitHardware(&uart, num, baud);
    return uart;
}


void uartTransmitByte(Uart* uart, uint8_t byte) { //发送一个字节
    USART_SendData(uart->handle, byte); //发送
	while(USART_GetFlagStatus(uart->handle, USART_FLAG_TXE) == RESET); //等待数据发送, 不需要手动置零标志位，写下一个数据时会自动置0（数据被发送出去后USART_FLAG_TXE（TX is Empty）会置1）
}

void uartTransmit(Uart* uart, uint8_t* data, uint32_t size) { //发送数据
    for(uint32_t i = 0; i < size; i++) {
		uartTransmitByte(uart, data[i]);
	}
}

void uartPrint(Uart* uart, const char* const format, ...) { //打印数据
    va_list args;
	va_start(args, format);

	char str[128] = {0};
	uint32_t i = 0;
	vsprintf(str, format, args); //格式化字符串转字符串
    while(i < 128 && str[i] != 0) {
        uartTransmitByte(uart, str[i++]);
    }
    uartTransmitByte(uart, 0); //发送结束符

	va_end(args);
}


void uartSetReceiveCb(Uart* uart, void (*callback)(uint8_t), uint8_t prePriority, uint8_t subPriority) { //设置接收回调
    __uartReceiveCb[uart->id] = callback; //设置回调
	
	USART_ITConfig(uart->handle, USART_IT_RXNE, ENABLE); //设置RXNE置1时中断（有数据被接收时USART_IT_RXNE（RX Not Empty）会置1）

    uint8_t channels[] = {USART1_IRQn, USART2_IRQn, USART3_IRQn};
	
	//初始化中断
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = channels[uart->id]; //指定中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = prePriority; //设置抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = subPriority; //设置响应优先级
	NVIC_Init(&NVIC_InitStructure);
}

void uartUseDefaultCb(Uart* uart, void (*callback)(char*, uint32_t), uint8_t prePriority, uint8_t subPriority) { //使用内置接收回调
    uartSetReceiveCb(uart, uartDefaultCb, prePriority, subPriority);
    __uartStringReceiveCb = callback;
}

void USART1_IRQHandler(void) { //中断函数
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET) { //判断有接收到数据
		if(__uartReceiveCb[0]) __uartReceiveCb[0](USART_ReceiveData(USART1)); //调用回调,传出数据
		USART_ClearITPendingBit(USART1, USART_IT_RXNE); //清除标志位
	}
}
void USART2_IRQHandler(void) {
	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
		if(__uartReceiveCb[1]) __uartReceiveCb[1](USART_ReceiveData(USART2));
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
}
void USART3_IRQHandler(void) {
	if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
		if(__uartReceiveCb[2]) __uartReceiveCb[2](USART_ReceiveData(USART3));
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
}
