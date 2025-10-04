#include "global.h"
#include "io.h"
#include "delay.h"
#include "hjos.h"
#include "nvic.h"
#include "iic.h"
#include "oled.h"

IicBus iicBus1, iicBus2;
Oled oled1, oled2;
uint8_t btn1State = 0, btn2State = 0;
HjosTcb* led1Tcb, *led2Tcb, *led3Tcb, *btn1Tcb, *btn2Tcb, *oled1Tcb, *oled2Tcb, *testTcb;

void led1Task(void* args) { //led1以1000ms为周期翻转
	Io led1 = ioCreate(2, 13, IO_PP);

	while(1) {
		ioSet(&led1, 0);
		hjosTaskDelay(500);
		ioSet(&led1, 1);
		hjosTaskDelay(500);
	}
}


void led2Task(void* args) { //led2以800ms为周期翻转
	Io led2 = ioCreate(0, 1, IO_PP);

	while(1) {
		ioSet(&led2, 0);
		hjosTaskDelay(400);
		ioSet(&led2, 1);
		hjosTaskDelay(400);
	}
}


void led3Task(void* args) { //led3以600ms为周期翻转
	Io led3 = ioCreate(0, 2, IO_PP);

	while(1) {
		ioSet(&led3, 0);
		hjosTaskDelay(300);
		ioSet(&led3, 1);
		hjosTaskDelay(300);
	}
}

void btn1Task(void* args) { //btn1状态检测
	Io btn1 = ioCreate(0, 3, IO_PU);

	while(1) {
		if(ioGet(&btn1) == 0) {
			btn1State = 1;
			hjosTaskDelay(20);
			while(ioGet(&btn1) == 0) hjosTaskDelay(1);
			btn1State = 0;
			hjosTaskDelay(20);
		}
		hjosTaskDelay(1);
	}
}

void btn2Task(void* args) { //btn2状态检测，并在按下时发生向消息队列发生消息
	Io btn2 = ioCreate(0, 4, IO_PU);
	HjosQueue* queue = args;

	while(1) {
		if(ioGet(&btn2) == 0) {
			btn2State = 1;
			uint8_t btn2Pushed = 1;
			hjosQueueSend(queue, &btn2Pushed);
			hjosTaskDelay(20);
			while(ioGet(&btn2) == 0) hjosTaskDelay(1);
			btn2State = 0;
			hjosTaskDelay(20);
		}
		hjosTaskDelay(1);
	}
}

void oled1Task(void* args) { //oled1任务，用于监测各项数值，按下btn2翻页
	uint64_t tick = 0;
	size_t num = 0;
	oledInit(&oled1, &iicBus1);

	uint8_t page = 1;
	HjosQueue* queue = args;
	
	while(1) {
		tick = hjosGetCurrentTick();

		uint8_t btn2Pushed = 0;
		uint8_t res = hjosQueueReceive(queue, &btn2Pushed, 0);
		if(res && btn2Pushed == 1) {
			page++;
			if(page > 4) page = 1;
		}

		if(page == 1) {
			uint16_t usage = hjosGetUsageAvg();
			oledClear(&oled1);
			oledPrint(&oled1, 0, 0, "num: %d", num);
			oledPrint(&oled1, 0, 1, "usage: %d.%d%%", usage / 10, usage % 10);
			oledPrint(&oled1, 0, 2, "btn1: %d, btn2: %d", btn1State, btn2State);
			oledPrint(&oled1, 0, 3, "idleMem: %d", heapGetIdle());
			oledUpdate(&oled1);
		}
		else if(page == 2) {
			oledClear(&oled1);
			oledPrint(&oled1, 0, 0, "stack idle:");
			oledPrint(&oled1, 0, 1, "l1: %d", hjosGetTaskStackIdle(led1Tcb));
			oledPrint(&oled1, 8, 1, "l2: %d", hjosGetTaskStackIdle(led2Tcb));
			oledPrint(&oled1, 0, 2, "b1: %d", hjosGetTaskStackIdle(btn1Tcb));
			oledPrint(&oled1, 8, 2, "b2: %d", hjosGetTaskStackIdle(btn2Tcb));
			oledPrint(&oled1, 0, 3, "o1: %d", hjosGetTaskStackIdle(oled1Tcb));
			oledPrint(&oled1, 8, 3, "o2: %d", hjosGetTaskStackIdle(oled2Tcb));
			oledUpdate(&oled1);
		}
		else if(page == 3) {
			oledClear(&oled1);
			oledPrint(&oled1, 0, 0, "testTask: %d", hjosGetTaskStackIdle(testTcb));
			oledPrint(&oled1, 0, 1, "taskSize: %d", hjosGetTaskSize());
			oledUpdate(&oled1);
		}
		else if(page == 4) {
			uint16_t usage = hjosGetUsage();
			uint16_t usageAvg = hjosGetUsageAvg();
			oledClear(&oled1);
			oledPrint(&oled1, 0, 0, "CPU: %d.%d%%", usage / 10, usage % 10);
			oledPrint(&oled1, 0, 1, "avg: %d.%d%%", usageAvg / 10, usageAvg % 10);
			oledUpdate(&oled1);
		}
		
		num++;
		hjosTaskDelayTo(tick + 100);
	}
}

void oled2Task(void* args) { //oled2任务，用于测试时间片切换
	uint64_t tick = 0;
	size_t num = 0;

	oledInit(&oled2, &iicBus2);
	
	while(1) {
		tick = hjosGetCurrentTick();

		oledClear(&oled2);
		oledPrint(&oled2, 0, 0, "num: %d", num);
		oledUpdate(&oled2);
		num++;
		hjosTaskDelayTo(tick + 100);
	}
}

void emptyTask(void* args) { //空任务
	while(1) {
		hjosTaskSwitch(100);
	}
}

void testTask(void* args) { //测试任务，用于测试任务的创建与删除，挂起与恢复
	uint8_t cnt = 0;
	while(1) {
		hjosTaskDelay(2000);
		HjosTcb* tcb1 = hjosTaskCreate("emptyTask1", emptyTask, 0, 64, 1);
		hjosTaskDelay(500);
		HjosTcb* tcb2 = hjosTaskCreate("emptyTask2", emptyTask, 0, 64, 1);
		hjosTaskDelay(500);
		hjosTaskDelete(tcb1);
		hjosTaskDelay(500);
		HjosTcb* tcb3 = hjosTaskCreate("emptyTask3", emptyTask, 0, 64, 1);
		hjosTaskDelay(500);
		hjosTaskDelete(tcb3);
		hjosTaskDelay(500);
		hjosTaskDelete(tcb2);

		hjosTaskSuspend(led3Tcb);
		hjosTaskDelay(2000);
		hjosTaskResume(led3Tcb);

		cnt++;

		if(cnt == 3) break;
	}

	hjosTaskDelete(0);
}


int main() {
	nvicSetGroup(2);
	
	iicBus1 = iicCreateSoftware(1, 6, 1, 7, 0);
	iicBus2 = iicCreateSoftware(1, 8, 1, 9, 0);
	
    hjosInit();

	HjosQueue* queue = hjosQueueCreate(8, 1);
	
	led1Tcb = hjosTaskCreate("led1Task", led1Task, 0, 64, 2);
	led2Tcb = hjosTaskCreate("led2Task", led2Task, 0, 64, 2);
	led3Tcb = hjosTaskCreate("led3Task", led3Task, 0, 64, 2);
	btn1Tcb = hjosTaskCreate("btn1Task", btn1Task, 0, 64, 3);
	btn2Tcb = hjosTaskCreate("btn2Task", btn2Task, queue, 64, 3);
	oled1Tcb = hjosTaskCreate("oled1Task", oled1Task, queue, 128, 1);
	oled2Tcb = hjosTaskCreate("oled2Task", oled2Task, 0, 128, 1);
	testTcb = hjosTaskCreate("testTask", testTask, 0, 64, 4);
	 
	SysTick_Config(HJOS_TICKTIM_RELOAD);
	hjosStart();
}