#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
#include "stdint.h"
typedef void* intr_handler;  //函数指针取一个别名
void idt_init(void);

//定义中断状态0表示关中断，1表示开中断
//枚举类型第一个默认0，后面的向后加
enum intr_status {		 
    INTR_OFF,			 
    INTR_ON		         
};

enum intr_status intr_get_status(void);
enum intr_status intr_set_status (enum intr_status);
enum intr_status intr_enable (void);
enum intr_status intr_disable (void);
#endif
