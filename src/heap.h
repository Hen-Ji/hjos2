#ifndef __HEAP_H
#define __HEAP_H

#include "hjosGlobal.h"

#define HEAP_ALIGNMENT 8 //8字节内存对齐
#define HEAP_SIZE HJOS_HEAP_SIZE //堆大小

void heapInit(); //堆初始化
void* heapNew(size_t size); //new
void* heapDelete(void*); //delete
void heapPrint(size_t* res, size_t len); //print
size_t heapGetIdle(); //获取所有空闲的堆的大小
size_t heapGetIdleMin(); //获取最小的空闲堆大小

#endif