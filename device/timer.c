#include "timer.h"
#include "io.h"
#include "print.h"
#include "interrupt.h"
#include "thread.h"
#include "debug.h"

#define IRQ0_FREQUENCY	   100
#define INPUT_FREQUENCY	   1193180//8253每秒1193180个节拍     
#define COUNTER0_VALUE	   INPUT_FREQUENCY / IRQ0_FREQUENCY//也就是说每秒发送100个信号
#define CONTRER0_PORT	   0x40  //计数器端口号是0x40
#define COUNTER0_NO	   0    //SC1和SC0位
#define COUNTER_MODE	   2    //采用比例发生器模式
#define READ_WRITE_LATCH   3 //高低字节都读取
#define PIT_CONTROL_PORT   0x43 //8253的控制端口

//初始化定时器8253  
//8253里一共有3个计数器  计数器0(0x40) 计数器1(0x41)  计数器2(0x42)    还有控制端口0x43
uint32_t ticks;          // ticks是内核自中断开启以来总时间片

/* 把操作的计数器counter_no、读写锁属性rwl、计数器模式counter_mode写入模式控制寄存器并赋予初始值counter_value */
static void frequency_set(uint8_t counter_port, uint8_t counter_no, uint8_t rwl, uint8_t counter_mode, uint16_t counter_value) {

   outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
   // 计数器0端口先写低8位后写高8位 
   outb(counter_port, (uint8_t)counter_value);

   outb(counter_port, (uint8_t)(counter_value >> 8));
}

/* 时钟的中断处理函数 */
static void intr_timer_handler(void) {
    struct task_struct* cur_thread = running_thread();
    ASSERT(cur_thread->stack_magic == 0x19960903); //通过魔数检查栈是否溢出

    cur_thread->elapsed_ticks++;	  
    ticks++;	  

    if (cur_thread->ticks == 0) {	  // 若进程时间片用完就开始调度
        
        schedule(); 
    } else {				
       cur_thread->ticks--;
    }
}
/* 初始化PIT8253 */
void timer_init() {
   put_str("timer_init start\n");
   //设置8253
   frequency_set(CONTRER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
   register_handler(0x20, intr_timer_handler);
   put_str("timer_init done\n");
}
