#include "hjosPort.h"

int32_t criticalCounter = 0;

void hjosCriticalEnter() { //关闭中断(只要进入临界区后就会关中断)
    if(criticalCounter == 0) __disable_irq();
    criticalCounter++;
}

void hjosCriticalExit() { //开启中断(只有退出第一级临界区后才会开启中断)
    criticalCounter--;
    if(criticalCounter == 0) __enable_irq();
}


void hjosSystickHandler() { //用于更新Tick并切换任务
	//注：此函数不能写过于复杂的代码（例如循环，判断等），否则可能导致寄存器的数据丢失（原因暂不详），大部分代码得在hjosTaskSwitchContext中实现
    extern uint64_t hjosCurrentTick;
    extern uint8_t hjosTickUpdated;

	hjosCurrentTick++; //下一个tick
	hjosTickUpdated = 1;
	
	hjosTaskYield(); //切换任务
}

__asm void hjosStartFirstTask( void ) { //用于在svc中启动第一个任务
    PRESERVE8 			//8字节对齐

	ldr r0, =0xE000ED08 //r0 = 0xE000ED08，在Cortex-M中，0xE000ED08是SCB_VTOR这个寄存器的地址, 里面存放的是向量表的起始地址
	ldr r0, [r0] 		//r0 = *r0，此时r0为向量表的起始地址
	ldr r0, [r0] 		//r0 = *r0，此时r0为向量表第一个元素的值，即MSP（主堆栈指针）的值

	msr msp, r0 		//msp = r0，将msp的值存到msp（好像有点多余）（=_=）
	
	cpsie i 			//开启全局中断
	cpsie f 			//开启异常中断
	dsb
	isb
	
	svc 0 				//产生SVC中断，接下来会执行SVC中断函数
	nop
	nop
}

__asm void hjosSvcHandler( void ) //用于启动第一个任务
{
    extern hjosCurrentTcb; //声明外部变量

	PRESERVE8

	ldr	r3, =hjosCurrentTcb	//r3 = &hjosCurrentTcb，将hjosCurrentTcb的地址赋值给r3
	ldr r1, [r3]			//r1 = *r3，即将hjosCurrentTcb的值赋值给r1
	ldr r0, [r1]			//r0 = *r1，此时r0为hjosCurrentTcb的值指向的值，即hjosTcb结构体的第一个元素的值，即此任务的栈顶指针
	ldmia r0!, {r4-r11}		//以r0为基地址，将栈中向上增长的8个字的内容加载到CPU寄存器r4~r11，同时r0也会跟着自增
	msr psp, r0				//psp = r0，将新的栈顶指针赋值给psp（进程堆栈指针）
	isb
	mov r0, #0 				//r0 = 0
	msr	basepri, r0 		//basepri = r0，即将basepri清零，即不关闭任何中断
	orr r14, #0xd 			//r14 |= 0xd0000000，使得硬件在退出时使用进程栈指针PSP完成出栈操作并返回后进入任务模式、返回Thumb状态
	bx r14 					//异常返回，这个时候出栈使用的是PSP指针，自动将栈中的剩下内容加载到CPU寄存器
}

__asm void hjosPendsvHandler( void ) //用于保存前一个任务的上下文并切换至下一个任务
{
	extern hjosCurrentTcb;
	extern hjosTaskSwitchContext;

	PRESERVE8

	mrs r0, psp //r0 = psp，当进入PendSVC Handler时，上一个任务运行的环境（xPSR，PC，R14，R12，R3，R2，R1，R0）这些CPU寄存器的值会自动存储到任务的栈中，剩下的r4~r11需要手动保存，同时PSP会自动更新（在更新之前PSP指向任务栈的栈顶）
	isb

	ldr	r3, =hjosCurrentTcb		//r3 = &hjosCurrentTcb
	ldr	r2, [r3]				//r2 = *r3

	stmdb r0!, {r4-r11}			//以r0作为基址，将CPU寄存器r4~r11的值存储到任务栈，同时更新r0的值（指针先递减，再操作）
	str r0, [r2]				//r0 = *r2，此时r0为当前任务的栈顶指针

	stmdb sp!, {r3, r14}		//将R3和R14临时压入栈（在整个系统中，中断使用的是主栈，栈指针使用的是MSP），避免调用函数时，数据被覆盖，导致数值丢失
	mov r0, #191			   //r0 = 191，即r0 = 0x1011 1111
	msr basepri, r0				//basepri = r0，即优先级高于或者等于11的中断都将被屏蔽
	dsb
	isb
	bl hjosTaskSwitchContext	//调用函数hjosTaskSwitchContext，切换上下文，具体为更新hjosCurrentTcb
	mov r0, #0					//r0 = 0
	msr basepri, r0				//basepri = r0，即打开所有中断
	ldmia sp!, {r3, r14}		//从主栈中恢复寄存器r3和r14的值

	ldr r1, [r3]				//r1 = *r3，此时r1指向新的任务
	ldr r0, [r1]				//r0 = *r1，此时r0为新的任务的栈顶指针
	ldmia r0!, {r4-r11}			//以r0为基地址，将栈中向上增长的8个字的内容加载到CPU寄存器r4~r11，同时r0也会跟着自增
	msr psp, r0					//psp = r0，等下异常退出时，会以psp作为基地址，将任务栈中剩下的内容自动加载到CPU寄存器
	isb
	bx r14						//异常返回
	nop
}