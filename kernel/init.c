#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"
//编译的时候用-I指定头文件的路径，就不需要相对路径


//内核初始化所用
void init_all()
{
	put_str("kernel init\n");
	idt_init();  //中断模块初始化
	timer_init();
	thread_init(); 
	mem_init();
}