#ifndef __KERNEL_GLOBAL_H
#define __KERNEL_GLOBAL_H
#include "stdint.h"

//GDT描述符属性  这部分内容和boot.inc差不多   之前是用汇编写的，现在换成C

#define	DESC_G_4K    1
#define	DESC_D_32    1//D设置1  表示32位
#define DESC_L	     0// 64位代码标记
#define DESC_AVL     0
#define DESC_P	     1
#define DESC_DPL_0   0
#define DESC_DPL_1   1
#define DESC_DPL_2   2
#define DESC_DPL_3   3


#define DESC_S_CODE	1//1表示存储段，0表示系统段
#define DESC_S_DATA	DESC_S_CODE
#define DESC_S_SYS	0
#define DESC_TYPE_CODE	8	//X C R A     x=1表示是代码段，R=1表示可读，C=0 表示非一致性代码段(这个和跳转时的特权级有关)  A accessed 显然没有访问过  
#define DESC_TYPE_DATA  2	// X E W A     X=0表示是数据段  E=0表示向上扩展  W=1 表示可写 A accessed 显然没有访问过 
#define DESC_TYPE_TSS   9	// B位为0,不忙


#define	 RPL0  0
#define	 RPL1  1
#define	 RPL2  2
#define	 RPL3  3

#define TI_GDT 0
#define TI_LDT 1

#define SELECTOR_K_CODE	   ((1 << 3) + (TI_GDT << 2) + RPL0)  //代码段
#define SELECTOR_K_DATA	   ((2 << 3) + (TI_GDT << 2) + RPL0)  //数据段
#define SELECTOR_K_STACK   SELECTOR_K_DATA                    //栈段
#define SELECTOR_K_GS	   ((3 << 3) + (TI_GDT << 2) + RPL0)  //显卡段
                                                              //第4个放TSS
#define SELECTOR_U_CODE	   ((5 << 3) + (TI_GDT << 2) + RPL3)  //用户代码段
#define SELECTOR_U_DATA	   ((6 << 3) + (TI_GDT << 2) + RPL3)  //用户数据段
#define SELECTOR_U_STACK   SELECTOR_U_DATA

#define GDT_ATTR_HIGH		 ((DESC_G_4K << 7) + (DESC_D_32 << 6) + (DESC_L << 5) + (DESC_AVL << 4))
#define GDT_CODE_ATTR_LOW_DPL3	 ((DESC_P << 7) + (DESC_DPL_3 << 5) + (DESC_S_CODE << 4) + DESC_TYPE_CODE)
#define GDT_DATA_ATTR_LOW_DPL3	 ((DESC_P << 7) + (DESC_DPL_3 << 5) + (DESC_S_DATA << 4) + DESC_TYPE_DATA)


//TSS描述符属性
#define TSS_DESC_D  0 //TSS的D为0
#define TSS_ATTR_HIGH ((DESC_G_4K << 7) + (TSS_DESC_D << 6) + (DESC_L << 5) + (DESC_AVL << 4) + 0x0)
#define TSS_ATTR_LOW ((DESC_P << 7) + (DESC_DPL_0 << 5) + (DESC_S_SYS << 4) + DESC_TYPE_TSS)
#define SELECTOR_TSS ((4 << 3) + (TI_GDT << 2 ) + RPL0)

struct gdt_desc {
   uint16_t limit_low_word;
   uint16_t base_low_word;
   uint8_t  base_mid_byte;
   uint8_t  attr_low_byte;
   uint8_t  limit_high_attr_high;
   uint8_t  base_high_byte;
}; 

//IDT描述符属性
#define	 IDT_DESC_P	 1 
#define	 IDT_DESC_DPL0   0
#define	 IDT_DESC_DPL3   3
#define	 IDT_DESC_32_TYPE     0xE   // 32位的门 1110
#define	 IDT_DESC_ATTR_DPL0  ((IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_32_TYPE)
#define	 IDT_DESC_ATTR_DPL3  ((IDT_DESC_P << 7) + (IDT_DESC_DPL3 << 5) + IDT_DESC_32_TYPE)


//eflags寄存器https://blog.csdn.net/jn1158359135/article/details/7761011
//IOPL(bits 12 and 13) [I/O privilege level field]   指示当前运行任务的I/O特权级(I/O privilege level)，正在运行任务的当前特权级(CPL)必须小于或等于I/O特权级才能允许访问I/O地址空间。这个域只能在CPL为0时才能通过POPF以及IRET指令修改。
#define EFLAGS_MBS	(1 << 1)	//MBS
#define EFLAGS_IF_1	(1 << 9)	// if为1,开中断
#define EFLAGS_IF_0	0		// if为0,关中断
#define EFLAGS_IOPL_3	(3 << 12)	// IOPL3 用户态
#define EFLAGS_IOPL_0	(0 << 12)	// IOPL0 内核态

#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))//http://blog.sina.com.cn/s/blog_875c95f40102uxjj.html   Linux向上取整除法  内核宏


#endif
