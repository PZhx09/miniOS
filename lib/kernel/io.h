
#ifndef __LIB_IO_H
#define __LIB_IO_H
#include "stdint.h"

//用C语言实现写入读取端口
//gcc用的汇编是AT&T，和之前intel的不太一样，现用现查把


// 向端口port写入一个字节
static inline void outb(uint16_t port, uint8_t data) {

 //a表示用寄存器al或ax或eax,对端口指定N表示0~255, d表示用dx存储端口号, 
 //%b0表示对应al,%w1表示对应dx   b是byte限定al，w是word限定dx
   asm volatile ( "outb %b0, %w1" : : "a" (data), "Nd" (port));    //后面的输入表示，data放在al，port在N(0-255)之间，放在dx里
}

// 将addr处起始的word_cnt个字写入端口port 
static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt) {

   //+表示此限制即做输入又做输出.
   //outsw是把ds:esi处的16位的内容写入port端口, 我们在设置段描述符时, 
   asm volatile ("cld; rep outsw" : "+S" (addr), "+c" (word_cnt) : "d" (port));

}

// 将从端口port读入的一个字节返回 
static inline uint8_t inb(uint16_t port) {
   uint8_t data;
   asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
   return data;
}

// 将从端口port读入的word_cnt个字写入addr 
static inline void insw(uint16_t port, void* addr, uint32_t word_cnt) {

   asm volatile ("cld; rep insw" : "+D" (addr), "+c" (word_cnt) : "d" (port) : "memory");

}

#endif
