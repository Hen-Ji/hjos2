#include "hjos.h"

uint64_t hjosCurrentTick = 0; //节拍
uint8_t hjosTickUpdated = 0; //tick更新标志
uint16_t hjosUsage = 0, hjosUsageAvg = 0; //使用率(单位0.1%)
uint64_t hjosUsageSum = 0; //500ms内使用率总和
HjosTcb* hjosCurrentTcb = 0; //当前正在运行的任务的任务控制块
HjosList* hjosCurrentList = 0; //HjosList<HjosTcb*>, 当前正在运行的任务的列表节点
size_t hjosCurrentIdx = 0; //当前正在运行的任务所处的优先级
uint8_t hjosIsIdle = 0; //任务是否空闲
uint64_t hjosIdleTime = 0; //任务开始空闲的时间
size_t hjosTaskSize = 0; //任务数量

HjosList hjosTcbList[HJOS_PRIORITY_LEVEL]; //HjosList<HjosTcb*>, 任务列表
HjosList hjosTcbDeleteList; //HjosList<HjosList<HjosTcb*>*>, 需要被删除的任务列表

HjosTcb* hjosIdleTcb; //空闲任务的任务控制块

void (*hjosIdleCallback)() = 0; //进入空闲状态前的函数回调
void (*hjosBusyCallback)() = 0; //进入忙碌状态前的函数回调
void (*hjosTaskSwitchCallback)() = 0; //任务切换前的函数回调
void (*hjosTaskSwitchedCallback)() = 0; //任务切换完成时的函数回调

static void hjosTaskExitError() {
    while(1); //一般是任务意外返回了，会执行到这里
}

void hjosIdleTask(void* task) {
	while(1) { //空闲任务
		if(hjosIsIdle == 0) { //刚进入空闲任务
			while(hjosTcbDeleteList.next) { //若有待删除的任务，则删除这些任务
				hjosCriticalEnter(); //进入临界区
				HjosList* node = (HjosList*)hjosTcbDeleteList.next->val; //取出待删除的任务节点
				HjosTcb* tcb = (HjosTcb*)node->val; //获取待删除的任务的任务控制块
				heapDelete(tcb->task); //释放任务资源
				heapDelete(tcb->stack); //释放任务栈资源
				heapDelete(tcb); //释放任务控制块本身
				hjosListErase(node); //在其所在的链表中删除此节点
				hjosListErase(hjosTcbDeleteList.next); //删除此待删除任务的节点
				hjosTaskSize--; //任务减1
				hjosCriticalExit(); //退出临界区
			}

			if(hjosIdleCallback) hjosIdleCallback(); //空闲前先调用回调

			hjosIdleTime = hjosCurrentTick*1000 + HJOS_TICKTIM_CNT*1000/HJOS_TICKTIM_RELOAD; //记录开始空闲的时间
			hjosIsIdle = 1;
		}
	}
}

HjosTcb* hjosTaskCreate(char* name, void (*cb)(void*), void* arg, uint32_t stackSize, uint8_t priority) {
    HjosTask* task = heapNew(sizeof(HjosTask)); //创建任务
    task->name = name;
    task->callback = cb;
    task->arg = arg;
    task->priority = priority;
    task->state = 0x00;
	task->stackSize = stackSize;
	task->delayToTick = 0;

    HjosTcb* tcb = heapNew(sizeof(HjosTcb)); //创建任务控制块
    tcb->task = task;
    tcb->stack = heapNew(stackSize * sizeof(size_t)); //创建任务栈
	for(size_t i = 0; i < stackSize; i++) tcb->stack[i] = HJOS_TASK_STACK_FILL; //将任务栈内预填充值，可用于判断空闲任务栈空间的历史最小值
    tcb->topOfStack = tcb->stack + stackSize-1;
    tcb->topOfStack = (size_t*)((size_t)tcb->topOfStack & (~(size_t)0x00000007)); //栈顶指针作向下8字节对齐

    tcb->topOfStack--;
    *tcb->topOfStack = 0x01000000; //xPSR第24位必须置1

    tcb->topOfStack--;
    *tcb->topOfStack = (size_t)cb & 0xfffffffeUL; //r15(PC)任务的入口地址

    tcb->topOfStack--;
    *tcb->topOfStack = (size_t)hjosTaskExitError; //r14(LR)保存子程序调用或异常处理返回的地址
    
    tcb->topOfStack--; //R12, R3, R2 and R1 默认初始化为0
    *tcb->topOfStack = 0;
    tcb->topOfStack--;
    *tcb->topOfStack = 0;
    tcb->topOfStack--;
    *tcb->topOfStack = 0;
    tcb->topOfStack--;
    *tcb->topOfStack = 0;

    tcb->topOfStack--;
    *tcb->topOfStack = (size_t)arg; //r0保存任务形参

    tcb->topOfStack -= 8; //异常发生时，手动加载到CPU寄存器的内容

    hjosListInsert(&hjosTcbList[priority], tcb); //任务控制块插入到头结点后面
	task->node = hjosTcbList[priority].next; //记录此节点到任务中，方便之后删除任务
	hjosTaskSize++; //任务加1

	return tcb;
}

void hjosTaskDelete(HjosTcb* tcb) {
	if(!tcb) tcb = hjosCurrentTcb; //如果tcb为0，则表示要删除当前任务
	tcb->task->state |= HJOS_TASK_STOP; //标记此任务已经停止
	hjosListInsert(&hjosTcbDeleteList, tcb->task->node); //将节点信息插入待删除任务的列表中
	if(tcb == hjosCurrentTcb) hjosTaskSwitch(); //如果是删除自己，则自动切换任务
}

void hjosInit() {
    for(int i = 0; i < HJOS_PRIORITY_LEVEL; i++) { //初始化任务列表
		hjosTcbList[i].val = 0;
		hjosTcbList[i].prev = 0;
		hjosTcbList[i].next = 0;
    }
	hjosTcbDeleteList.val = 0; //初始化需要被删除的任务列表
	hjosTcbDeleteList.prev = 0;
	hjosTcbDeleteList.next = 0;

	hjosIdleTcb = hjosTaskCreate("idle", hjosIdleTask, 0, HJOS_IDLE_TASK_STACK_SIZE, 0); //先创建空闲任务
}

void hjosStartScheduler() {
    hjosStartFirstTask();
}

void hjosStart() {
    hjosCurrentIdx = HJOS_PRIORITY_LEVEL - 1; //切换至第一个任务
	hjosCurrentList = &hjosTcbList[hjosCurrentIdx];
	hjosTaskSwitchContext();

    hjosStartScheduler(); //启动任务调度器
}

void hjosTaskSwitchContext() {
	if(hjosTickUpdated) { //tick更新了
		if(hjosBusyCallback) hjosBusyCallback(); //进入忙碌状态前调用回调函数

		hjosCriticalEnter(); //进入临界区

		hjosTickUpdated = 0;

		if(hjosCurrentTcb != hjosIdleTcb) { //若触发定时器中断后，当前运行的任务不是空闲任务，说明当前CPU占用率为100%
			hjosUsage = 1000;
		}
		else {
			hjosUsage = hjosCurrentTick*1000 - hjosIdleTime; //计算空闲时间
			if(hjosUsage > 1000) hjosUsage = 1000;
			hjosUsage = 1000 - hjosUsage; //计算占用率
		}

		hjosUsageSum += hjosUsage;
		if(hjosCurrentTick % (HJOS_TICK_FREQ/2) == 0) { //每500ms更新一次平均占用率
			hjosUsageAvg = hjosUsageSum / (HJOS_TICK_FREQ/2);
			hjosUsageSum = 0;
		}

		if(hjosCurrentTcb->task->node->next) { //当前任务所处的优先级中还有其他任务，则轮转一次
			HjosList* currNode = hjosCurrentTcb->task->node; //当前任务栈节点
			HjosList* nextNode = currNode->next; //下一个任务栈节点
			HjosList* lastNode = nextNode; //当前优先级中最后一个任务栈节点
			while(lastNode->next) lastNode = lastNode->next;
			HjosList* currHead = &hjosTcbList[hjosCurrentIdx]; //当前优先级的头节点
			HjosList* firstNode = currHead->next; //当前优先级中第一个任务栈节点

			currHead->next = nextNode; //使nextNode为第一个节点，currNode为最后一个节点，lastNode的下一个节点为firstNode
			nextNode->prev = currHead;
			lastNode->next = firstNode;
			firstNode->prev = lastNode;
			currNode->next = 0;
			//示例：head -> first -> curr -> next -> last -> NULL
			//变换后: head -> next -> last -> first -> curr -> NULL
		}
		
		hjosCurrentIdx = HJOS_PRIORITY_LEVEL - 1; //从最高优先级的任务开始
		hjosCurrentList = &hjosTcbList[hjosCurrentIdx];
			
		hjosCriticalExit(); //退出临界区
	}

	if(hjosTaskSwitchCallback) hjosTaskSwitchCallback(); //切换任务前调用回调函数

	hjosCriticalEnter();

	if(hjosCurrentTcb) hjosCurrentTcb->task->state &= ~HJOS_TASK_BUSY; //取消当前任务的忙状态

	while(1) {
		if(hjosCurrentList->next == 0) { //同一优先级的任务执行完了
			while(hjosCurrentIdx > 0) {
				hjosCurrentIdx--; //不同优先级，高优先级的任务优先于低优先级的任务运行
				if(hjosTcbList[hjosCurrentIdx].next) { //此优先级有任务，则选择此优先级，若此优先级没有任务，则往更低的优先级里面找
					hjosCurrentList = &hjosTcbList[hjosCurrentIdx];
					break;
				}
			}
		}

		if(hjosCurrentList->next) { //同一优先级，任务按顺序执行
			hjosCurrentList = hjosCurrentList->next; //切换至下一个任务
		}
		
		hjosCurrentTcb = (HjosTcb*)hjosCurrentList->val; //切换至此任务
		if(!(hjosCurrentTcb->task->state & (HJOS_TASK_STOP | HJOS_TASK_SUSPEND)) && hjosCurrentTcb->task->delayToTick <= hjosCurrentTick) break; //若此任务不被标记停止或挂起，并且延时时间到或没有延时，则执行此任务，否则继续切换至下一个任务
	}

	if(hjosCurrentTcb) hjosCurrentTcb->task->state |= HJOS_TASK_BUSY; //给当前任务忙状态
	hjosIsIdle = 0; //CPU处于忙状态

	hjosCriticalExit();

	if(hjosTaskSwitchedCallback) hjosTaskSwitchedCallback(); //任务切换完成后调用回调函数
}

void hjosTaskSuspend(HjosTcb* tcb) {
	if(tcb == 0) tcb = hjosCurrentTcb;
	tcb->task->state |= HJOS_TASK_SUSPEND; //挂起标志位置位
	if(tcb == hjosCurrentTcb) hjosTaskSwitch(); //如果是挂起自己，则自动切换任务
}

void hjosTaskResume(HjosTcb* tcb) {
	if(tcb == 0) tcb = hjosCurrentTcb;
	tcb->task->state &= ~HJOS_TASK_SUSPEND; //挂起标志位复位
}

void hjosTaskYield() {
    *((volatile uint32_t *)0xe000ed04) = 1UL << 28UL; //将PendSV的悬起位置1，当没有其他中断运行的时候响应PendSV中断
    __dsb(15); //完成数据同步隔离和指令同步隔离，确保开启PendSV
    __isb(15);
	//对于72MHz主频下的STM32F103C8T6，进行一次任务切换耗时约5-10us，tick更新后的第一次任务切换耗时为10-15us
}

void hjosTaskSwitch() {
	hjosTaskYield();
}

void hjosTaskDelay(uint64_t tick) {
	hjosCurrentTcb->task->delayToTick = hjosCurrentTick + tick; //设置延时到的时间
	hjosTaskSwitch(); //切换任务
}

void hjosTaskDelayTo(uint64_t tick) {
	hjosCurrentTcb->task->delayToTick = tick;
	hjosTaskSwitch();
}

size_t hjosGetTaskStackIdle(HjosTcb* tcb) {
	size_t* curr = tcb->stack;
	size_t* end = curr + tcb->task->stackSize-1;
	while(curr < end) {
		if(*curr == HJOS_TASK_STACK_FILL) curr++; //若值为HJOS_TASK_STACK_FILL，则认为此数据未被使用，继续往下找，直到找到有被使用过的数据（缺点：当有数据与HJOS_TASK_STACK_FILL相同时可能不准）
		else break;
	}
	size_t idleSize1 = (size_t)curr - (size_t)tcb->stack; //保存得到的值

	size_t idleSize2 = 0;
	if(tcb->topOfStack > tcb->stack) idleSize2 = (size_t)tcb->topOfStack - (size_t)tcb->stack; //用栈顶指针位置得到当前的空闲块大小（缺点：只能获取在这行代码运行时，任务的空闲块大小）

	return (idleSize1 < idleSize2 ? idleSize1 : idleSize2) / sizeof(size_t); //两种方法取最小值，返回空闲块的大小，但仍有可能不是历史最小值
}

uint64_t hjosGetCurrentTick() {
	return hjosCurrentTick;
}

uint16_t hjosGetUsage() {
	return hjosUsage;
}

uint16_t hjosGetUsageAvg() {
	return hjosUsageAvg;
}

size_t hjosGetTaskSize() {
	return hjosTaskSize;
}

HjosQueue* hjosQueueCreate(size_t maxSize, size_t itemSize) {
	HjosQueue* queue = heapNew(sizeof(HjosQueue)); //创建消息队列

	queue->begin = 0;
	queue->size = 0;
	queue->maxSize = maxSize;
	queue->itemSize = itemSize;
	queue->buffer = heapNew(maxSize * itemSize);

	return queue;
}
uint8_t hjosQueueSend(HjosQueue* queue, void* item) {
	hjosCriticalEnter();
	if(queue->size >= queue->maxSize) {
		hjosCriticalExit();
		return 0; //队列已满时返回
	}
	size_t idx = (queue->begin + queue->size) % queue->maxSize, itemSize = queue->itemSize; //计算消息存放的位置
	queue->size++; //队列长度加1
	hjosCriticalExit();

	for(size_t i = 0; i < itemSize; i++) { //存放消息
		queue->buffer[idx*itemSize + i] = ((uint8_t*)item)[i];
	}

	return 1; //发送完成
}
uint8_t hjosQueueReceive(HjosQueue* queue, void* item, uint64_t timeout) {
	if(timeout != 0) {
		uint64_t tick = hjosCurrentTick;
		while(tick + timeout > hjosCurrentTick) { //等待timeout个tick
			if(queue->size == 0) hjosTaskSwitch(); //没有消息则切换任务
			else break;
		}
	}

	hjosCriticalEnter();
	if(queue->size == 0) {
		hjosCriticalExit();
		return 0; //队列为空时返回
	}
	queue->size--; //队列不为空，则长度减1
	size_t idx = queue->begin, itemSize = queue->itemSize;
	queue->begin = (queue->begin + 1) % queue->maxSize; //队头移一格
	hjosCriticalExit();

	for(size_t i = 0; i < itemSize; i++) { //有消息则将消息存放至item
		((uint8_t*)item)[i] = queue->buffer[idx*itemSize + i];
	}

	return 1;
}

void hjosListInsert(HjosList* node, void* val) {
    hjosCriticalEnter(); //进入临界区

    HjosList* newNode = heapNew(sizeof(HjosList));
    newNode->val = val;
    newNode->prev = node; //插入到此节点之后
    newNode->next = node->next;
    node->next = newNode;
    if(newNode->next) newNode->next->prev = newNode;

    hjosCriticalExit(); //退出临界区
}

void hjosListErase(HjosList* node) {
    hjosCriticalEnter(); //进入临界区

    if(node->prev) node->prev->next = node->next; //连接前后两个节点
    if(node->next) node->next->prev = node->prev;
    heapDelete(node); //释放当前节点的资源

    hjosCriticalExit(); //退出临界区
}


void hjosSetIdleCallback(void (*p)()) {
	hjosIdleCallback = p;
}
void hjosSetBusyCallback(void (*p)()) {
	hjosBusyCallback = p;
}
void hjosSetSwitchCallback(void (*p)()) {
	hjosTaskSwitchCallback = p;
}
void hjosSetSwitchedCallback(void (*p)()) {
	hjosTaskSwitchedCallback = p;
}