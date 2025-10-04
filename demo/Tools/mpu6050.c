#include "mpu6050.h"

#define MPU6050_ADDRESS 0x68 // 110 1000

#define	MPU6050_SMPLRT_DIV		0x19	//陀螺仪采样率，典型值：0x07(125Hz)
#define	MPU6050_CONFIG			0x1A	//低通滤波频率，典型值：0x06(5Hz)
#define	MPU6050_GYRO_CONFIG		0x1B	//陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define	MPU6050_ACCEL_CONFIG	0x1C	//加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define	MPU6050_ACCEL_XOUT_H	0x3B
#define	MPU6050_ACCEL_XOUT_L	0x3C
#define	MPU6050_ACCEL_YOUT_H	0x3D
#define	MPU6050_ACCEL_YOUT_L	0x3E
#define	MPU6050_ACCEL_ZOUT_H	0x3F
#define	MPU6050_ACCEL_ZOUT_L	0x40
#define	MPU6050_TEMP_OUT_H		0x41
#define	MPU6050_TEMP_OUT_L		0x42

#define	MPU6050_GYRO_XOUT_H		0x43
#define	MPU6050_GYRO_XOUT_L		0x44	
#define	MPU6050_GYRO_YOUT_H		0x45
#define	MPU6050_GYRO_YOUT_L		0x46
#define	MPU6050_GYRO_ZOUT_H		0x47
#define	MPU6050_GYRO_ZOUT_L		0x48

#define	MPU6050_PWR_MGMT_1		0x6B	//电源管理1
#define	MPU6050_PWR_MGMT_2		0x6C	//电源管理2
#define	MPU6050_WHO_AM_I		0x75	//IIC地址寄存器(默认数值0x68，只读)

IicDevice mpu6050Device;

void mpu6050Write(uint8_t addr, uint8_t data) { //写数据
	uint8_t datas[2] = {addr, data};
	iicTransmit(&mpu6050Device, datas, 2);
}

uint8_t mpu6050Read(uint8_t addr) { //读数据
	uint8_t data = addr;
	uint8_t buffer = 0;
	iicReceive(&mpu6050Device, &data, 1, &buffer, 1);
	return buffer;
}

void mpu6050Init(IicBus *bus) {
	mpu6050Device = iicCreateDevice(bus, MPU6050_ADDRESS);
	
	mpu6050Write(MPU6050_PWR_MGMT_1, 0x01); //电源管理1，取消睡眠模式，使用陀螺仪时钟
	mpu6050Write(MPU6050_PWR_MGMT_2, 0x00); //电源管理2，默认0x00
	mpu6050Write(MPU6050_SMPLRT_DIV, 0x09); //10分频
	mpu6050Write(MPU6050_CONFIG, 0x06); //设置低通滤波器
	mpu6050Write(MPU6050_GYRO_CONFIG, 0x10); //配置陀螺仪，不自测，最大8g
	mpu6050Write(MPU6050_ACCEL_CONFIG, 0x10); //配置加速度计，不自测，最大1000度/s
}

void mpu6050Get(Vector *a, Vector *g) { //获取六轴数据(单位分别转为 g 和 deg/s)
	uint8_t h, l;
	uint16_t v;
	
	h = mpu6050Read(MPU6050_ACCEL_XOUT_H);
	l = mpu6050Read(MPU6050_ACCEL_XOUT_L);
	a->x = (float)(int16_t)((h << 8) | l) /4096;
	
	h = mpu6050Read(MPU6050_ACCEL_YOUT_H);
	l = mpu6050Read(MPU6050_ACCEL_YOUT_L);
	a->y = (float)(int16_t)((h << 8) | l) /4096;
	
	h = mpu6050Read(MPU6050_ACCEL_ZOUT_H);
	l = mpu6050Read(MPU6050_ACCEL_ZOUT_L);
	a->z = (float)(int16_t)((h << 8) | l) /4096;
	
	h = mpu6050Read(MPU6050_GYRO_XOUT_H);
	l = mpu6050Read(MPU6050_GYRO_XOUT_L);
	g->x = (float)(int16_t)((h << 8) | l) / 32.768;

	h = mpu6050Read(MPU6050_GYRO_YOUT_H);
	l = mpu6050Read(MPU6050_GYRO_YOUT_L);
	g->y = (float)(int16_t)((h << 8) | l) / 32.768;
	
	h = mpu6050Read(MPU6050_GYRO_ZOUT_H);
	l = mpu6050Read(MPU6050_GYRO_ZOUT_L);
	g->z = (float)(int16_t)((h << 8) | l) / 32.768;
}