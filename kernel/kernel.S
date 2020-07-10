[bits 32]
%define ERROR_CODE nop
%define ZERO push 0

extern put_str;要用到之前写的put_str
extern idt_table
section .data

intr_str db "interrupt occur!",0xD,0

global intr_entry_table

intr_entry_table:

%macro VECTOR 2   ;汇编中的宏    %macro 宏名  参数个数
section .text
intr%1entry:
    %2     ;错误码，CPU不自动压入的话，就手动压入
    push ds
    push es
    push fs
    push gs
    pushad


    ;发送中断处理结束信号EOI
    mov al,0x20
    out 0xA0,al
    out 0x20,al
    push %1

    call [idt_table+%1*4]
    jmp intr_exit


section .data
    dd intr%1entry  ;存放中断入口程序的地址   最后形成一个数组
%endmacro

;中断入口有很多个，但是出口只有一个
section .text
global intr_exit
intr_exit:	     

   add esp, 4			   ; 中断号
   popad
   pop gs
   pop fs
   pop es
   pop ds
   add esp, 4			   ; error_code
   iret


;0-19是处理器固定的异常类型，例如0就是除0异常
;20-31是intel保留的  
;OS能自己设计的是从32  0x20开始的
VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO 
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO 
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE 
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO 
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO 
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO 
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE 
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO 
;外部中断   8259A
VECTOR 0x20,ZERO  ;时钟中断
VECTOR 0x21,ZERO  ;键盘中断
VECTOR 0x22,ZERO  ;级联用的
VECTOR 0x23,ZERO  ;串口2对应的入口
VECTOR 0x24,ZERO  ;串口1对应的入口
VECTOR 0x25,ZERO  ;并口2对应的入口
VECTOR 0x26,ZERO  ;软盘对应的入口
VECTOR 0x27,ZERO  ;并口1对应的入口
VECTOR 0x28,ZERO  ;实时时钟对应的入口
VECTOR 0x29,ZERO  ;重定向
VECTOR 0x2a,ZERO  ;保留
VECTOR 0x2b,ZERO  ;保留
VECTOR 0x2c,ZERO  ;ps/2鼠标
VECTOR 0x2d,ZERO  ;fpu浮点单元异常
VECTOR 0x2e,ZERO  ;硬盘
VECTOR 0x2f,ZERO  ;保留