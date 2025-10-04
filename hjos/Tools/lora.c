#include "lora.h"

SpiDevice loraDevice;
Io loraRst, loraBusy;
Exti loraDioExti;
uint32_t loraFreq;
uint8_t loraBandwidth, loraSpreadingFactor, loraCodingRate;
uint8_t loraMode;
uint8_t loraTransmitting;

uint8_t loraTransmitBuffer[260] = {0}, loraReceiveBuffer[260] = {0};

void (*loraTransmitCallback)(uint8_t) = 0;
void (*loraReceiveCallback)(uint8_t*, uint8_t) = 0;

uint8_t loraIsBusy() {
	return ioGet(&loraBusy);
}

uint8_t loraGetDio() {
    return ioGet(&loraDioExti.io);
}

void loraSpiTransmitBytes(uint8_t* bytes, uint16_t size) {
	while(loraIsBusy()); //等待设备空闲
	spiTransmit(&loraDevice, bytes, size);

	for(uint16_t i = 0; i < 100; i++); //需要延时至少600ns，让设备反应
}

void loraSpiSwapBytes(uint8_t* data, uint8_t* receiveBuffer, uint32_t size) {
	while(loraIsBusy()); //等待设备空闲
	spiSwap(&loraDevice, data, receiveBuffer, size);

	for(uint16_t i = 0; i < 100; i++); //需要延时至少600ns，让设备反应
}

void loraSpiTransmit(uint8_t byte) {
	loraSpiTransmitBytes(&byte, 1);
}

void loraSpiTransmitCommands(uint8_t* commands, uint16_t size) {
    for(uint16_t i = 0; i < size; i++) {
		uint8_t s = commands[i];
		if(s == 0) break;
		loraSpiTransmitBytes(&commands[i+1], s);
		//i += s + 1; //我要把这行代码钉在我的床头TAT
		i += s;
	}
}

LoraStatus loraGetStatus() {
    LoraStatus res;
    uint8_t data[2] = {0xC0};
	uint8_t buf[2] = {0};
	loraSpiSwapBytes(data, buf, 2); //获取状态码
	
    res.mode = (buf[1] & 0x70) >> 4; //获取当前模式
	res.mode = (buf[1] & 0x0E) >> 1; //获取当前状态
    return res;
}

LoraIrqStatus loraGetIrqStatus() {
    LoraIrqStatus res;
    uint8_t data[4] = {0x12};
	uint8_t buf[4] = {0};
	loraSpiSwapBytes(data, buf, 4); //获取状态码

    res.txDone = buf[3] & 0x01; //获取中断状态
    res.rxDone = (buf[3] >> 1) & 0x01;
    res.preambleDetected = (buf[3] >> 2) & 0x01;
    res.headerValid = (buf[3] >> 4) & 0x01;
    res.headerErr = (buf[3] >> 5) & 0x01;
    res.crcErr = (buf[3] >> 6) & 0x01;
    res.cadDone = buf[3] >> 7;
    res.cadDetected = buf[2] & 0x01;
    res.timeout = (buf[2] >> 1) & 0x01;
    return res;
}

void loraClearIrqStatus(uint16_t mask) {
	uint8_t data[] = {0x02, mask >> 8, mask & 0xFF};
	loraSpiTransmitBytes(data, sizeof(data));
}

uint8_t loraReadRegister(uint16_t addr) {
	uint8_t data[5] = {0x1D, addr >> 8, addr & 0xFF};
	uint8_t buf[5] = {0};

	loraSpiSwapBytes(data, buf, 5);
	return buf[4];
}

void loraDioTriggered() {
    LoraIrqStatus status = loraGetIrqStatus(); //获取irq标志
    loraClearIrqStatus(0xFFFF); //清除所有irq标志
    if(loraMode == LORA_TX) { //发送模式
        loraTransmitting = 0;
        if(loraTransmitCallback) {
            if(status.txDone) loraTransmitCallback(0); //发送成功
            if(status.timeout) loraTransmitCallback(1); //发送超时
        }
    }
    else if(loraMode == LORA_RX) { //接收模式
        if(loraReceiveCallback) {
            if(status.rxDone && !status.crcErr && !status.headerErr) { //接收成功
                uint8_t data[4] = {0x13};
                uint8_t bufferStatus[4] = {0};
                loraSpiSwapBytes(data, bufferStatus, 4); // 获取RxBufferStatus，bufferStatus[2]为接收到的数据长度，bufferStatus[3]为获取接收到的数据在Lora模块的buffer中的起始位置

                loraTransmitBuffer[0] = 0x1E;
                loraTransmitBuffer[1] = bufferStatus[3];
                loraSpiSwapBytes(loraTransmitBuffer, loraReceiveBuffer, bufferStatus[2] + 3); //获取数据

                loraReceiveCallback(&loraReceiveBuffer[3], bufferStatus[2]); //将数据传出去
            }
        }
    }
}

void loraInit(SpiBus* bus, uint8_t csPort, uint8_t csPin, uint8_t rstPort, uint8_t rstPin, uint8_t busyPort, uint8_t busyPin, uint8_t dioPort, uint8_t dioPin, uint8_t dioPrePriority, uint8_t dioSubPriority) {
	loraDevice = spiCreateDevice(bus, csPort, csPin); //初始化SPI设备
	loraRst = ioCreate(rstPort, rstPin, IO_PP); //初始化引脚
	loraBusy = ioCreate(busyPort, busyPin, IO_FL);
    loraDioExti = extiCreate(dioPort, dioPin, IO_FL, EXTI_R);
    extiStart(&loraDioExti, loraDioTriggered, dioPrePriority, dioSubPriority); //开启中断

    loraFreq = 470500000; //默认射频频率为470.5MHz
    loraBandwidth = LORA_BW_125KHZ; //默认带宽为125KHz
    loraSpreadingFactor = LORA_SF_9; //默认扩散因子为9
    loraCodingRate = LORA_CR_4_5; //默认编码率为4/5

	ioSet(&loraRst, 0); //初始化LLCC68
	delay(20);
	ioSet(&loraRst, 1);
	delay(10);

	loraSetStandbyMode(); //设置standby
}

void loraSetFreq(uint32_t freq) {
    loraFreq = freq;
}
void loraSetBandwidth(uint8_t bw) {
    loraBandwidth = bw;
}
void loraSetSpreadingFactor(uint8_t sf) {
    loraSpreadingFactor = sf;
}
void loraSetCodingRate(uint8_t cr) {
    loraCodingRate = cr;
}

void loraSetMode(uint8_t mode) {
    if(mode == LORA_STANDBY) loraSetStandbyMode();
    else if(mode == LORA_TX) loraSetTransmitMode();
    else if(mode == LORA_RX) loraSetReceiveMode();
}
void loraSetStandbyMode() {
    uint8_t datas[] = {0x80, 0x00}; //设置STDBY_RC
    loraSpiTransmitBytes(datas, sizeof(datas));
    loraMode = LORA_STANDBY;
}
void loraSetTransmitMode() {
    loraSetStandbyMode();
	
	uint32_t freq = (uint32_t)((uint64_t)loraFreq * (1 << 25) / 32000000); //计算射频频率
	uint8_t txClampConfig = loraReadRegister(0x08D8);
	txClampConfig |= 0x1E; // fix bug: 15.2 Better Resistance of the LLCC68 Tx to Antenna Mismatch
	uint8_t initParams[] = {
		2, 0x8A, 0x01, // 设置为Lora模式
		5, 0x0D, 0x07, 0x40, 0x34, 0x44, // 作为公共网络(0x3444)设置同步字
		2, 0x96, 0x01, // 使用DC-DC
		5, 0x86, (freq >> 24), (freq >> 16) % 0xFF, (freq >> 8) % 0xFF, freq % 0xFF, // 设置射频频率
		5, 0x95, 0x04, 0x07, 0x00, 0x01, // 设置放大器功率为+22dBm（最大值）
		3, 0x8E, 22, 0x02, // 设置TxParams：设置输出功率为+22dBm，斜坡时间为40us（0x02）
		4, 0x0D, 0x08, 0xD8, txClampConfig, // 优化放大器阈值，以达到期望的功率
		3, 0x8F, 0x00, 0x00, // 设置Tx和Rx的基地址均为0
		5, 0x8B, // 设置modulationParams
			loraSpreadingFactor, // 设置扩散因子
			loraBandwidth, // 设置带宽
			loraCodingRate, // 设置编码率
			0x00, // 关闭低速率优化(0x00: off; 0x01: on)
    	9, 0x08, 0xFF, 0xFF, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, // 开启所有中断，并将Timeout和TxDone中断映射到DIO1上
		0
	};
  	loraSpiTransmitCommands(initParams, sizeof(initParams));
    
    loraMode = LORA_TX;
}

void loraSetReceiveMode() {
    loraSetStandbyMode();
	
	uint32_t freq = (uint32_t)((uint64_t)loraFreq * (1 << 25) / 32000000); //计算射频频率
	uint8_t reg = loraReadRegister(0x0736);
	reg |= 0x04; // fix bug: 15.4 Optimizing the Inverted IQ Operation
	uint8_t initParams[] = {
		2, 0x8A, 0x01, // 设置为Lora模式
		5, 0x0D, 0x07, 0x40, 0x34, 0x44, // 作为公共网络(0x3444)设置同步字
		2, 0x96, 0x01, // 使用DC-DC
		5, 0x86, (freq >> 24), (freq >> 16) % 0xFF, (freq >> 8) % 0xFF, freq % 0xFF, // 设置射频频率
		3, 0x8F, 0x00, 0x00, // 设置Tx和Rx的基地址均为0
		5, 0x8B, // 设置modulationParams
			loraSpreadingFactor, // 设置扩散因子
			loraBandwidth, // 设置带宽
			loraCodingRate, // 设置编码率
			0x00, // 关闭低速率优化(0x00: off; 0x01: on)
    	7, 0x8C, // 设置 packetParams
    		0x00, 0x08, // 设置前导码长度为 8
    	  	0x00, // 设置为不定长数据表
      		0xFF, // 设置有效载荷长度为255（最大值）
      		0x01, // 使用CRC校验
      		0x00, // 使用标准IQ设置
		4, 0x0D, 0x07, 0x36, reg, // bit 2 must be 1 when using standard IQ polarity
    	9, 0x08, 0xFF, 0xFF, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, // 开启所有中断，并将Timeout和RxDone中断映射到DIO1上
		4, 0x82, 0xFF, 0xFF, 0xFF, // 启动Rx模式并设置超时时间，0xFFFFFF表示一直接收
		0
	};
  	loraSpiTransmitCommands(initParams, sizeof(initParams));

    loraMode = LORA_RX;
}

uint8_t loraTransmit(uint8_t *bytes, uint8_t size, uint32_t timeout) {
    if(loraTransmitting) return 1; //上一条数据还在传输，直接返回
    loraTransmitting = 1;

    loraTransmitBuffer[0] = 0x0E; // 写入缓冲指令
	loraTransmitBuffer[1] = 0x00; // 设置偏移量为0
	for(int i = 0; i < size; i++) loraTransmitBuffer[i+2] = bytes[i];
	loraSpiTransmitBytes(loraTransmitBuffer, size + 2); //写入缓冲

	uint32_t t = (float)1000 * timeout / 15.625; // 计算发送超时时间
  	uint8_t params[] = {
    	7, 0x8C, // 设置 packetParams
    		0x00, 0x08, // 设置前导码长度为 8
    	  	0x00, // 设置为不定长数据表
      		size, // 设置有效载荷长度
      		0x01, // 使用CRC校验
      		0x00, // 使用标准IQ设置
		4, 0x83, (t >> 16) % 0xFF, (t >> 8) % 0xFF, t % 0xFF, // 启动Tx模式并设置超时时间，0x000000表示一直发送，直到发送完成
    	0
  	};
  	loraSpiTransmitCommands(params, sizeof(params));
    return 0; //已经将数据发送给设备，准备传输
}
void loraTransmitSync(uint8_t *bytes, uint8_t size, uint32_t timeout) {
    loraTransmit(bytes, size, timeout);
    while(loraIsTransmitting); //等待传输完成
}

uint8_t loraPrint(uint32_t timeout, const char* const format, ...) {
    va_list args;
	va_start(args, format);

	char str[128] = {0};
	uint32_t size = 0;
	vsprintf(str, format, args); //格式化字符串转字符串
    while(size < 128 && str[size] != 0) {
        size++;
    }
    
    uint8_t res = loraTransmit((uint8_t*)str, size, timeout); //传输

	va_end(args);
    return res;
}
void loraPrintSync(uint32_t timeout, const char* const format, ...) {
    va_list args;
	va_start(args, format);

	char str[128] = {0};
	uint32_t size = 0;
	vsprintf(str, format, args); //格式化字符串转字符串
    while(size < 128 && str[size] != 0) {
        size++;
    }
    
    loraTransmitSync((uint8_t*)str, size, timeout); //传输

	va_end(args);
}

void loraSetTransmitCb(void (*p)(uint8_t result)) {
    loraTransmitCallback = p;
}
void loraSetReceiveCb(void (*p)(uint8_t* data, uint8_t size)) {
    loraReceiveCallback = p;
}
uint8_t loraIsTransmitting() {
    return loraTransmitting;
}