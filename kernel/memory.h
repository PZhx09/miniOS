#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"

//页表项和页目录项都是4字节，一个页4KB所以只要20位就能表示，其他的都是属性
#define	 PG_P_1	  1	// 页表项或页目录项存在
#define	 PG_P_0	  0	// 页表项或页目录项不存在
#define	 PG_RW_R  0	// R/W 属性位值, 读/执行
#define	 PG_RW_W  1<<1	// R/W 属性位值, 读/写/执行
#define	 PG_US_S  0	// U/S 属性位值, 系统级
#define	 PG_US_U  1<<2	// U/S 属性位值, 用户级
//页目录项和页表项里存在的肯定是物理地址，访问页目录表和页表肯定也都是用物理地址，不然的话就陷入逻辑死循环了



//内存池标记,用于判断用哪个内存池
enum pool_flags {
   PF_KERNEL = 1,    // 内核内存池
   PF_USER = 2	     // 用户内存池
};

//虚拟地址池，用来管理虚拟地址
struct virtual_addr {
   struct bitmap vaddr_bitmap; // 虚拟地址用到的位图结构 
   uint32_t vaddr_start;       // 虚拟地址起始地址
};

extern struct pool kernel_pool, user_pool;
void mem_init(void);
void* get_kernel_pages(uint32_t pg_cnt);
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt);
void malloc_init(void);
uint32_t* pte_ptr(uint32_t vaddr);
uint32_t* pde_ptr(uint32_t vaddr);
#endif
