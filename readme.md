# HJOS移植

1. 将相应的 .h 和 .c 文件（必要文件：hjos, hjosGlobal, hjosPort, heap）加载进项目，并添加 `hjos.h` 头文件

2. 开启SVC中断，PendSV中断（默认应该已经开启）和SysTick中断并将中断优先级设置为最低，然后将其中断函数重定向至对应函数hjosSvcHandler，hjosPendsvHandler和hjosSystickHandler，比如在hjosGlobal.h中修改这些宏定义：

   ```c
   #define hjosSvcHandler SVC_Handler //重定向
   #define hjosPendsvHandler PendSV_Handler
   #define hjosSystickHandler SysTick_Handler
   ```

   注意，不能使用类似以下的写法，否则可能出BUG：

   ```c
   void SVC_Handler(void) { //这样写可能会出BUG！！！
       hjosSvcHandler();
   }
   ```

   如果已经定义了类似SVC_Handler，PendSV_Handler，SysTick_Handler的函数，记得注释掉

3. main函数写法：

   ```c
   int main() {
       NVIC_INIT(); //设置中断优先级
       
       hjosInit(); //初始化HJOS
       
       ... //其他初始化代码
           
       hjosTaskCreate(...); //创建任务
       
       TIMER_START(FREQ); //启动节拍定时器，以一定频率进入中断
       
       hjosStart(); //启动HJOS
       
       while(1); //正常情况下，程序不会运行到这里
   }
   ```

4. 在 hjosGlobal.h 中，导入全局头文件，需要包含以下数据类型：uint8_t, uint16_t uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t, size_t

5. 在 hjosPort.c 中，修改需要进行适配的函数

6. 编写demo程序保证HJOS已移植完毕



STM32F103C8T6的示例代码如下：

```c
#include "global.h" //一些其他的头文件
#include "io.h" //用于操作io口
#include "hjos.h" //hjos头文件
#include "nvic.h" //用于中断优先级分组

void task1(HjosTask* task) { //IO电平翻转任务
	Io led = ioCreate(2, 13, IO_PP); //初始化PC13，设置推挽输出

	while(1) { //while循环，不应该退出
		ioSet(&led, 0); //io口设置低电平
		hjosTaskDelay(500); //延时500tick
		ioSet(&led, 1); //io口设置高电平
		hjosTaskDelay(500); //延时500tick
	}
}

int main() {
	nvicSetGroup(2); //优先级分组，两位抢占，两位响应
	
    hjosInit(); //初始化HJOS
    
    hjosTaskCreate("task1", task1, 0, 128, 1); //创建任务
    
    SysTick_Config(TICKTIM_RELOAD); //使用SysTick，并设置中断频率为1000Hz
    
    hjosStart(); //启动HJOS
}
```



# HJOS概述

**HJOS2**（简称 **HJOS** ）是一个抢占式内核的实时系统，实现了不同优先级间的抢占式调度和相同优先级间的时间片轮转。

HJOS相关文件为：`hjos.h`, `hjos.c`, `hjosGlobal.h`, `hjosGlobal.c`, `hjosPort.h`, `hjosPort.c` 共6个文件。以下为各文件的说明：

- **hjos**

  HJOS的核心文件，实现了任务相关的函数，消息队列相关的函数等大部分常用函数。一般不需要更改

- **hjosGlobal**

  HJOS的配置文件，系统需要用到的大部分参数都可以在这里配置

- **hjosPort**

  HJOS针对不同类型的单片机实现的接口文件，比如临界区的进入与退出，第一个任务的启动，以及三个中断的中断服务程序。通过更改这些函数，可以在不同类型的单片机上进行移植

此外，HJOS还依赖 **heap** 库，用于实现堆内存的动态分配和释放，用户可直接使用提供的 `heap.h`, `heap.c` 文件，也可仿照文件自行编写 `heapNew`, `heapDelete` 这两个函数提供给HJOS使用