#ifndef __W25Q64_H
#define __W25Q64_H

#include "global.h"
#include "spi.h"

void w25q64Init(SpiBus *bus, uint8_t csPort, uint8_t csPin);
void w25q64WriteEnable();
uint8_t w25q64IsBusy();
void w25q64Erase(uint32_t addr);
void w25q64Write(uint32_t addr, uint8_t* arr, uint32_t size); //write前要先erase对应的块
void w25q64Read(uint32_t addr, uint8_t* arr, uint32_t size);
void w25q64ReadID(uint8_t *mid, uint16_t *did);

#endif