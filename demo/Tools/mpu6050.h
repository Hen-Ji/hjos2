#ifndef __MPU6050_H
#define __MPU6050_H

#include "global.h"
#include "io.h"
#include "iic.h"

/**
  * @brief  单精度浮点类型的三维向量
  * @param  x x轴
  * @param  y y轴
  * @param  z z轴
  */
typedef struct _Vector {
    float x, y, z;
}Vector;

/**
  * @brief  MPU6050初始化
  * @param  bus MPU6050需要挂载到的IIC总线
  */
void mpu6050Init(IicBus *bus);

/**
  * @brief  MPU6050获取加速度 (单位: g) 和角速度 (单位: deg/s)
  * @param  a 加速度信息保存在这
  * @param  g 角速度信息保存在这
  */
void mpu6050Get(Vector *a, Vector *g);

#endif