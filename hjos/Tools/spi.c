#include "spi.h"

void spiDelay(uint32_t x) {
    for(uint32_t i = 0; i < x; i++);
}

void spiInitHardware(SpiBus *bus, uint8_t num, uint32_t freq) {
    bus->busy = 0;
    bus->isHardWare = 1;
    bus->freq = freq;

    SPI_TypeDef* spis[] = {SPI1, SPI2};
    bus->handle = spis[num-1];

    int32_t x = freq == 0 ? 0 : CLOCK_FREQ / freq; //计算分频, 为0默认跑满速率
    if(num == 2) x /= 2; //SPI2挂载在APB1，其时钟频率比APB1的频率少一半，要达到相同的速率分频需要除2
    uint16_t bauds;
    if(x <= 2) bauds = SPI_BaudRatePrescaler_2;
    else if(x <= 4) bauds = SPI_BaudRatePrescaler_4;
    else if(x <= 8) bauds = SPI_BaudRatePrescaler_8;
    else if(x <= 16) bauds = SPI_BaudRatePrescaler_16;
    else if(x <= 32) bauds = SPI_BaudRatePrescaler_32;
    else if(x <= 64) bauds = SPI_BaudRatePrescaler_64;
    else if(x <= 128) bauds = SPI_BaudRatePrescaler_128;
    else bauds = SPI_BaudRatePrescaler_256;

    if(num == 1) { //SPI1
        bus->miso = ioCreate(0, 6, IO_PU); //miso上拉输入
        bus->mosi = ioCreate(0, 7, IO_AFPP); //mosi复用推挽输出
        bus->sclk = ioCreate(0, 5, IO_AFPP); //sclk复用推挽输出

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //SPI1在APB2上, SPI2在APB1上
    }
    else if(num == 2) { //SPI2
        bus->miso = ioCreate(1, 14, IO_PU); //miso上拉输入
        bus->mosi = ioCreate(1, 15, IO_AFPP); //mosi复用推挽输出
        bus->sclk = ioCreate(1, 13, IO_AFPP); //sclk复用推挽输出

        RCC_APB2PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    }

    SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //主模式
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //双线全双工
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //8位数据帧
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //高位先行
	SPI_InitStructure.SPI_BaudRatePrescaler = bauds; //分频
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; //时钟极性为默认低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; //第一个边沿采样（上升沿时获取），CPOL和CPHA共同组成模式0
	SPI_InitStructure.SPI_NSS= SPI_NSS_Soft; //软件片选
	SPI_InitStructure.SPI_CRCPolynomial = 7; //CRC校验数，不用这个，随便填
	SPI_Init(bus->handle, &SPI_InitStructure);
	
	SPI_Cmd(bus->handle, ENABLE);
}
SpiBus spiCreateHardware(uint8_t num, uint32_t freq) {
    SpiBus bus;
    spiInitHardware(&bus, num, freq);
    return bus;
}

void spiInitSoftware(SpiBus *bus, uint8_t misoPort, uint8_t misoPin, uint8_t mosiPort, uint8_t mosiPin, uint8_t sclkPort, uint8_t sclkPin, uint32_t cycle) {
    bus->busy = 0;
    bus->isHardWare = 0;
    bus->miso = ioCreate(misoPort, misoPin, IO_PU); //miso上拉输入
    bus->mosi = ioCreate(mosiPort, mosiPin, IO_PP); //mosi推挽输出
    bus->sclk = ioCreate(sclkPort, sclkPin, IO_PP); //sclk推挽输出
    bus->cycle = cycle;

    ioSet(&bus->mosi, 0); //默认均为低电平
    ioSet(&bus->sclk, 0);
}

SpiBus spiCreateSoftware(uint8_t misoPort, uint8_t misoPin, uint8_t mosiPort, uint8_t mosiPin, uint8_t sclkPort, uint8_t sclkPin, uint32_t cycle) {
    SpiBus bus;
    spiInitSoftware(&bus, misoPort, misoPin, mosiPort, mosiPin, sclkPort, sclkPin, cycle);
    return bus;
}

void spiAddDevice(SpiDevice* device, SpiBus* bus, uint8_t csPort, uint8_t csPin) {
    device->bus = bus;
    device->cs = ioCreate(csPort, csPin, IO_PP);
    ioSet(&device->cs, 1); //初始化为高电平
}

SpiDevice spiCreateDevice(SpiBus* bus, uint8_t csPort, uint8_t csPin) {
    SpiDevice device;
    spiAddDevice(&device, bus, csPort, csPin);
    return device;
}

uint8_t spiBusStart(SpiBus* bus, Io* cs) {
    if(bus->busy) return 1;
    bus->busy = 1;
    ioSet(cs, 0); //连接到从机的片选置低电平，表示要开始给从机交换数据
    return 0;
}

uint8_t spiBusStop(SpiBus* bus, Io* cs) {
    if(!bus->busy) return 1;
    ioSet(cs, 1); //片选置高电平，表示交换结束
    bus->busy = 0;
    return 0;
}

uint8_t spiBusSwapByte(SpiBus* bus, uint8_t byte) {
    if(bus->isHardWare) { //硬件SPI，使用非连续模式
        while(SPI_I2S_GetFlagStatus(bus->handle, SPI_I2S_FLAG_TXE) != SET); //等待寄存器TX为空
        SPI_I2S_SendData(bus->handle, byte); //写数据
        while(SPI_I2S_GetFlagStatus(bus->handle, SPI_I2S_FLAG_RXNE) != SET); //等待寄存器RX非空，即收到数据
        return SPI_I2S_ReceiveData(bus->handle); //读数据返回
    }
    else { //软件SPI
        if(bus->cycle) { //加延时
            for(int i = 0; i < 8; i++) {
                ioSet(&bus->mosi, byte & 0x80);
                spiDelay(bus->cycle);
                byte <<= 1;
                ioSet(&bus->sclk, 1);
                spiDelay(bus->cycle);
                if(ioGet(&bus->miso)) byte |= 0x01;
                spiDelay(bus->cycle);
                ioSet(&bus->sclk, 0);
                spiDelay(bus->cycle);
            }
        }
        else { //不加延时
            for(int i = 0; i < 8; i++) {
                ioSet(&bus->mosi, byte & 0x80); //发送数据，高位先行
                byte <<= 1; //左移一位
                ioSet(&bus->sclk, 1); //SCK置1，准备接收数据
                if(ioGet(&bus->miso)) byte |= 0x01; //接收数据，由从机的高位读到主机的低位，再通过移位移到高位，节省一个变量
                ioSet(&bus->sclk, 0); //SCK置0，准备交换下一个数据或准备结束
            }
        }
    }
    return byte;
}
void spiBusSwap(SpiBus* bus, uint8_t* data, uint8_t* receiveBuffer, uint32_t size) {
    if(data == 0) { //只接收
        for(uint32_t i = 0; i < size; i++) {
            receiveBuffer[i] = spiBusSwapByte(bus, 0xFF); //发送0xFF交换
        }
    }
    else if(receiveBuffer == 0) { //只发送
        for(uint32_t i = 0; i < size; i++) {
            spiBusSwapByte(bus, data[i]);
        }
    }
    else { //接收和发送
        for(uint32_t i = 0; i < size; i++) {
            receiveBuffer[i] = spiBusSwapByte(bus, data[i]);
        }
    }
}

uint8_t spiTransmit(SpiDevice* device, uint8_t* data, uint32_t size) {
    if(spiBusStart(device->bus, &device->cs)) return 1;

    spiBusSwap(device->bus, data, 0, size);

    spiBusStop(device->bus, &device->cs);
    return 0;
}

uint8_t spiReceive(SpiDevice* device, uint8_t* data, uint32_t size, uint8_t* receiveBuffer, uint32_t receiveSize) {
    if(spiBusStart(device->bus, &device->cs)) return 1;

    spiBusSwap(device->bus, data, 0, size); //发送数据
    spiBusSwap(device->bus, 0, receiveBuffer, receiveSize); //接收数据
    
    spiBusStop(device->bus, &device->cs);
    return 0;
}

uint8_t spiSwap(SpiDevice* device, uint8_t* data, uint8_t* receiveBuffer, uint32_t size) {
    if(spiBusStart(device->bus, &device->cs)) return 1;

    spiBusSwap(device->bus, data, receiveBuffer, size);
    
    spiBusStop(device->bus, &device->cs);
    return 0;
}