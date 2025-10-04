#include "heap.h"


typedef struct _HeapList {
    size_t size; //此内存块大小
    struct _HeapList *next; //下一个空闲块
}HeapList;


uint8_t heap[HEAP_SIZE]; //堆
HeapList* heapHead; //堆空间空闲块链表头节点,指向第一个空闲块
HeapList* heapEnd; //堆空间空闲块链表尾节点
size_t heapListSize; //结构体的大小
size_t heapIdleMem, heapIdleMemMin; //剩余内存

void heapInit() { //堆初始化
    hjosCriticalEnter(); //进入临界区

    heapListSize = sizeof(HeapList); //结构体大小

    heapHead = (HeapList*)(heap); //头结点放进堆头
    heapEnd = (HeapList*)(&(heap[HEAP_SIZE-heapListSize])); //尾结点放进堆尾
    HeapList* heapFirstIdleNode = (HeapList*)(heap) + 1; //第一个空闲块节点紧靠在头结点后

    heapFirstIdleNode->size = HEAP_SIZE - 2*heapListSize; //头尾节点都占了一定空间
    heapFirstIdleNode->next = heapEnd; //头节点指向第一个空闲块节点，第一个空闲块节点指向尾结点
    heapHead->size = heapListSize;
    heapHead->next = heapFirstIdleNode;
    heapEnd->size = heapListSize;
    heapEnd->next = 0;

    heapIdleMem = heapFirstIdleNode->size; //记录总的可用内存
    heapIdleMemMin = heapIdleMem;

    hjosCriticalExit(); //退出临界区
}

void* heapNew(size_t size) {
    hjosCriticalEnter(); //进入临界区
    
    if(!heapEnd) heapInit(); //没有初始化就先初始化

    size += heapListSize; //此节点还要占一定空间
    size = (((size - 1) / HEAP_ALIGNMENT) + 1) * HEAP_ALIGNMENT; //内存对齐
    if(size > heapIdleMem) { //size不能超过剩余空间
        hjosCriticalExit(); //退出临界区
        return 0;
    };

    HeapList *prev = heapHead, *node;
    while (prev->next && prev->next->size < size)  prev = prev->next; //获得第一个空间足够大的空闲的堆空间
    node = prev->next;
    if (!node) { //找不到就返回0
        hjosCriticalExit(); //退出临界区
        return 0;
    };

    if(node->size > size + heapListSize) { //有足够的空间加新节点
        HeapList* newNode = (HeapList*)((size_t)node + size); //加新节点
        newNode->next = node->next; //prev指向newnode，newnode指向下一个空闲块节点
        prev->next = newNode;
        newNode->size = node->size - size; //设置占用的空间
        node->size = size;
    
        heapIdleMem -= size; //减少剩余空间
    }
    else { //没有足够的空间加新节点就直接把多出来的空间合并掉，能减少内存碎片
        prev->next = node->next;
        heapIdleMem -= node->size;
    }

    heapIdleMemMin = heapIdleMemMin > heapIdleMem ? heapIdleMem : heapIdleMemMin; //最小值

    hjosCriticalExit(); //退出临界区
    return (void*)((size_t)node + heapListSize); //返回new好的地址
}

void* heapDelete(void* addr) {
    if(!addr || (size_t)addr < (size_t)heapHead || (size_t)addr > (size_t)heapEnd) return 0;
    hjosCriticalEnter(); //进入临界区
    HeapList* node = (HeapList*)((size_t)addr - heapListSize); //获取节点信息
    heapIdleMem += node->size; //回收内存

    HeapList* prev = heapHead;
    while(prev->next != heapEnd && (size_t)prev->next < (size_t)node) prev = prev->next; //找前面的节点

    node->next = prev->next; //prev指向node，node指向下一个空闲块节点
    prev->next = node;

    if (prev == heapHead) prev = prev->next; //头节点不参与合并，直接指向下一个空闲块节点
    else if ((size_t)prev + prev->size == (size_t)node) { //与左边的合并
        prev->size += node->size;
        prev->next = node->next;
    }

    if (prev->next != heapEnd && (size_t)prev + prev->size == (size_t)(prev->next)) { //与右边的合并，且尾结点不参与合并
        prev->size += prev->next->size;
        prev->next = prev->next->next;
    }

    hjosCriticalExit(); //退出临界区
    return 0;
}

void heapPrint(size_t* res, size_t len) { //打印堆的使用情况
    hjosCriticalEnter(); //进入临界区

    HeapList* node = heapHead->next;
    size_t i = 0;
    while(i+1 < len && node != heapEnd) {
        res[i++] = (size_t)node - (size_t)heapHead; //res = [第一个空闲块开始，第一个空闲块结束，第二个空闲块开始，...]
        res[i++] = (size_t)node - (size_t)heapHead + node->size;
        node = node->next;
    }
    if(i < len) res[i++] = 0;

    hjosCriticalExit(); //退出临界区
}


size_t heapGetIdle() {
    return heapIdleMem;
}
size_t heapGetIdleMin() {
    return heapIdleMemMin;
}