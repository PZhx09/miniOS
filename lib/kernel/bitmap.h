#ifndef __LIB_KERNEL_BITMAP_H
#define __LIB_KERNEL_BITMAP_H
#include "global.h"
#define BITMAP_MASK 1
struct bitmap {
   uint32_t btmp_bytes_len;//位图结构的长度
   uint8_t* bits;          //指针，指向位图结构的首地址  uint8_t表示指向一个字节，8位，方便后面比较
};

void bitmap_init(struct bitmap* btmp);//位图初始化，整个位图的的内容都初始化为0
bool bitmap_scan_test(struct bitmap* btmp, uint32_t bit_idx);//判断bit_idx位，是1返回true，是0返回false
int bitmap_scan(struct bitmap* btmp, uint32_t cnt);//在位图中申请连续cnt个位，成功返回下标，否则返回-1
void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value);//将bit_idx设位置value    
#endif
