#include "iic.h"

uint16_t iicWaitEvent(IicBus* bus, uint32_t event) { //等待事件，超时退出
    uint16_t i = 0x0A00; //2560
	while(I2C_CheckEvent(bus->handle, event) != SUCCESS && i > 0) i--;
	return i;
}

void iicDelay(uint32_t x) {
    for(uint32_t i = 0; i < x; i++);
}

void iicInitHardware(IicBus* bus, uint8_t num, uint32_t freq) { //初始化硬件IIC
    bus->isHardware = 1;
    bus->busy = 0;
    bus->freq = (freq == 0 ? 100000 : freq); //freq为0则使用100khz的频率，保证稳定性
    bus->cycle = 0;
    
    if(num == 1) { //目前只支持I2C1，I2C2
        bus->scl = ioCreate(1, 6, IO_AFOD); //初始化引脚，均为复用开漏输出
        bus->sda = ioCreate(1, 7, IO_AFOD);
        
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE); //使能时钟
        bus->handle = I2C1; //放handle
    }
    else if(num == 2) {
        bus->scl = ioCreate(1, 10, IO_AFOD);
        bus->sda = ioCreate(1, 11, IO_AFOD);
        
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
        bus->handle = I2C2;
    }


    I2C_InitTypeDef I2C_InitStructure;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C; //使用I2C
	I2C_InitStructure.I2C_ClockSpeed = bus->freq; //设置的频率（可接收0~100KHz为标准模式，100KHz~400KHz为快速模式）
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; //设置快速模式时，SCL的占空比为2比1（由于上拉电阻是若上拉，低电平到高电平是时缓慢上升的，故需要等SDA稳定时拉高SCL才能比较准确地读出数据）
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable; //默认需要应答
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; //stm32作为从机时用7位地址
	I2C_InitStructure.I2C_OwnAddress1 = 0x00; //stm32作为从机时的地址
	I2C_Init(bus->handle, &I2C_InitStructure);
	
	I2C_Cmd(bus->handle, ENABLE); //使能
}

void iicInitSoftware(IicBus* bus, uint8_t sclPort, uint8_t sclPin, uint8_t sdaPort, uint8_t sdaPin, uint32_t cycle) { //初始化软件IIC
    bus->isHardware = 0;
    bus->busy = 0;
    bus->cycle = cycle;

    bus->scl = ioCreate(sclPort, sclPin, IO_OD); //初始化引脚，均为开漏输出
    bus->sda = ioCreate(sdaPort, sdaPin, IO_OD);
    ioSet(&bus->scl, 1); //初始化电平，都为高表示总线空闲
    ioSet(&bus->sda, 1);
    if(bus->cycle) iicDelay(bus->cycle);
}

IicBus iicCreateHardware(uint8_t num, uint32_t freq) {
    IicBus bus;
    iicInitHardware(&bus, num, freq);
    return bus;
}

IicBus iicCreateSoftware(uint8_t sclPort, uint8_t sclPin, uint8_t sdaPort, uint8_t sdaPin, uint32_t cycle) {
    IicBus bus;
    iicInitSoftware(&bus, sclPort, sclPin, sdaPort, sdaPin, cycle);
    return bus;
}

uint8_t iicBusStart(IicBus* bus) { //总线启动
    if(bus->busy) return 1; //若总线忙，则立即退出
    bus->busy = 1; //设置忙状态
    if(bus->isHardware) {
        while(I2C_GetFlagStatus(bus->handle, I2C_FLAG_BUSY)); //等待总线不忙
        I2C_GenerateSTART(bus->handle, ENABLE); //生成起始条件(会等待之前的数据已经发送完成再产生)
        iicWaitEvent(bus, I2C_EVENT_MASTER_MODE_SELECT); //等待EV5事件（stm32变为主模式）
    }
    else {
        uint32_t cycle = bus->cycle;
        ioSet(&bus->sda, 1);
        if(cycle) iicDelay(cycle);
        ioSet(&bus->scl, 1); //先释放sda, 再释放scl，保证释放时，sda为高电平
        if(cycle) iicDelay(cycle);
        ioSet(&bus->sda, 0);
        if(cycle) iicDelay(cycle);
        ioSet(&bus->scl, 0);
        if(cycle) iicDelay(cycle);
    }
    return 0;
}

uint8_t iicBusRestart(IicBus* bus) { //总线重启
    if(!bus->busy) return 1; //这时总线应处于忙状态
    if(bus->isHardware) {
        I2C_GenerateSTART(bus->handle, ENABLE); //生成起始条件(会等待之前的数据已经发送完成再产生)
        iicWaitEvent(bus, I2C_EVENT_MASTER_MODE_SELECT); //等待EV5事件（stm32变为主模式）
    }
    else {
        uint32_t cycle = bus->cycle;
        ioSet(&bus->sda, 1);
        if(cycle) iicDelay(cycle);
        ioSet(&bus->scl, 1); //先释放sda, 再释放scl，保证释放时，sda为高电平
        if(cycle) iicDelay(cycle);
        ioSet(&bus->sda, 0);
        if(cycle) iicDelay(cycle);
        ioSet(&bus->scl, 0);
        if(cycle) iicDelay(cycle);
    }
    return 0;
}

uint8_t iicBusStop(IicBus* bus) { //总线停止
    if(!bus->busy) return 1; //这时总线应处于忙状态
    if(bus->isHardware) {
        iicWaitEvent(bus, I2C_EVENT_MASTER_BYTE_TRANSMITTED); //等待EV8-2事件（字节发送完毕）
        I2C_GenerateSTOP(bus->handle, ENABLE); //生成停止条件
    }
    else {
        uint32_t cycle = bus->cycle;
        ioSet(&bus->sda, 0);
        if(cycle) iicDelay(cycle);
        ioSet(&bus->scl, 1);
        if(cycle) iicDelay(cycle);
        ioSet(&bus->sda, 1);
        if(cycle) iicDelay(cycle);
    }
    bus->busy = 0; //取消忙状态
    return 0;
}


uint8_t iicBusTransmitByte(IicBus* bus, uint8_t byte) { //发一个字节
    if(!bus->busy) return 1;

    if(bus->isHardware) { //硬件IIC
        I2C_SendData(bus->handle, byte); //发送数据
        iicWaitEvent(bus, I2C_EVENT_MASTER_BYTE_TRANSMITTED); //等待EV8-2事件（字节发送完毕）
    }
    else { //软件IIC
        uint32_t j, cycle = bus->cycle;
        if(cycle) { //需要加延时
            for(j = 0; j < 8; j++) {
                ioSet(&bus->sda, byte & (0x80 >> j));
                iicDelay(cycle);
                ioSet(&bus->scl, 1);
                iicDelay(cycle);
                ioSet(&bus->scl, 0);
                iicDelay(cycle);
            }
	        ioSet(&bus->sda, 1); //释放sda
            iicDelay(cycle);
            ioSet(&bus->scl, 1);
            iicDelay(cycle);
            bus->ack = ioGet(&bus->sda);
            iicDelay(cycle);
            ioSet(&bus->scl, 0);
            iicDelay(cycle);
        }
        else { //不需要加延时
            for(j = 0; j < 8; j++) { //高位现行
                ioSet(&bus->sda, byte & (0x80 >> j));
                ioSet(&bus->scl, 1);
                ioSet(&bus->scl, 0);
            }
	        ioSet(&bus->sda, 1); //释放sda
            ioSet(&bus->scl, 1);
            bus->ack = ioGet(&bus->sda); //读ack
            ioSet(&bus->scl, 0);
        }
    }

    return 0;
}

uint8_t iicBusTransmit(IicBus* bus, uint8_t* data, uint32_t size) { //发数据
    if(!bus->busy) return 1;
    for(uint32_t i = 0; i < size; i++) iicBusTransmitByte(bus, data[i]);

    return 0;
}

uint8_t iicBusTransmitAddr(IicBus* bus, uint8_t addr, uint8_t direction) { //发地址
    if(!bus->busy) return 1;

    if(bus->isHardware) { //硬件IIC
        I2C_Send7bitAddress(bus->handle, addr << 1, direction); //发送目标地址，并且确定是发送还是接收(0/I2C_Direction_Transmitter为发，1/I2C_Direction_Receiver为收)
        if(direction == IIC_WRITE) iicWaitEvent(bus, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED); //等待EV6事件（地址发送）
        else iicWaitEvent(bus, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED); //也是等待EV6事件（地址发送）
    }
    else { //软件IIC
        addr = (addr << 1) + direction; //合并地址和读写信号
        iicBusTransmitByte(bus, addr); //直接发出去
    }

    return 0;
}

uint8_t iicBusReceiveByte(IicBus* bus, uint8_t* byte, uint8_t ack) {
    if(!bus->busy) return 1;
    *byte = 0;

    if(bus->isHardware) { //硬件IIC
        if(ack == IIC_NACK) I2C_AcknowledgeConfig(bus->handle, DISABLE); //配置是否非应答
        iicWaitEvent(bus, I2C_EVENT_MASTER_BYTE_RECEIVED); //等待EV7事件（数据接收完毕）
        *byte = I2C_ReceiveData(bus->handle); //返回数据
        I2C_AcknowledgeConfig(bus->handle, ENABLE); //重新enable，一半下次接收
    }
    else { //软件IIC
        uint32_t j, cycle = bus->cycle;
        if(cycle) { //需要加延时
            ioSet(&bus->sda, 1); //释放sda
            iicDelay(cycle);
            for(j = 0; j < 8; j++) {
                ioSet(&bus->scl, 1);
                iicDelay(cycle);
                if(ioGet(&bus->sda) == 1) *byte |= 0x80 >> j;
                iicDelay(cycle);
                ioSet(&bus->scl, 0);
                iicDelay(cycle);
            }
            ioSet(&bus->sda, ack);
            iicDelay(cycle);
            ioSet(&bus->scl, 1);
            iicDelay(cycle);
            ioSet(&bus->scl, 0);
            iicDelay(cycle);
        }
        else { //不需要加延时
            ioSet(&bus->sda, 1); //释放sda
            for(j = 0; j < 8; j++) { //高位现行
                ioSet(&bus->scl, 1); //上升沿接收
                if(ioGet(&bus->sda) == 1) *byte |= 0x80 >> j;
                ioSet(&bus->scl, 0);
            }
            ioSet(&bus->sda, ack); //发送应答
            ioSet(&bus->scl, 1);
            ioSet(&bus->scl, 0);
        }
    }

    return 0;
}

uint8_t iicBusReceive(IicBus* bus, uint8_t* data, uint32_t size) { //接收
    if(!bus->busy) return 1;

    for(uint32_t i = 0; i < size-1; i++) iicBusReceiveByte(bus, &data[i], IIC_ACK); //发送应答，表示要继续传数据
    iicBusReceiveByte(bus, &data[size-1], IIC_NACK); //发送非应答，表示不需要继续传数据了


    return 0;
}

uint8_t iicBusReadAck(IicBus* bus) { //获取ack
    if(bus->isHardware) {
        return 0; //硬件IIC没找到获取ACK的函数，先放着吧
    }
    else {
        return bus->ack;
    }
}

void iicAddDevice(IicDevice* device, IicBus* bus, uint8_t address) { //挂载设备
    device->bus = bus;
    device->address = address; //存7位地址(写到低7位)
}

IicDevice iicCreateDevice(IicBus* bus, uint8_t address) {
    IicDevice device;
    iicAddDevice(&device, bus, address);
    return device;
}

uint8_t iicTransmit(IicDevice* device, uint8_t* data, uint32_t size) { //传数据
    if(iicBusStart(device->bus)) return 1; //使用总线，若总线忙则退出

    iicBusTransmitAddr(device->bus, device->address, IIC_WRITE); //发送地址以及写信号
    iicBusTransmit(device->bus, data, size); //发送数据

    iicBusStop(device->bus); //结束
    return 0;
}

uint8_t iicReceive(IicDevice* device, uint8_t* data, uint32_t size, uint8_t* receiveBuffer, uint32_t receiveSize) { //接收数据
    if(iicBusStart(device->bus)) return 1; //使用总线，若总线忙则退出

    iicBusTransmitAddr(device->bus, device->address, IIC_WRITE); //发送地址以及写信号
    iicBusTransmit(device->bus, data, size); //发送数据

    iicBusRestart(device->bus); //重新启动

    iicBusTransmitAddr(device->bus, device->address, IIC_READ); //发送地址以及读信号
    iicBusReceive(device->bus, receiveBuffer, receiveSize); //接收数据

    iicBusStop(device->bus); //结束
    return 0;
}

