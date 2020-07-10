#ifndef __USERPROG_PROCESS_H 
#define __USERPROG_PROCESS_H 
#include "thread.h"
#include "stdint.h"
#define default_prio 31 //默认特权级
#define USER_STACK3_VADDR  (0xc0000000 - 0x1000)//用户空间栈
#define USER_VADDR_START 0x8048000//遵循linux标准，好像是早期386上的某个类unix系统用了这个值，就一直延续下来了，没有找到是否有特殊意义
void process_execute(void* filename, char* name);
void start_process(void* filename_);
void process_activate(struct task_struct* p_thread);
void page_dir_activate(struct task_struct* p_thread);
uint32_t* create_page_dir(void);
void create_user_vaddr_bitmap(struct task_struct* user_prog);
#endif
