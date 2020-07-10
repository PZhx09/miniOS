#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H
#include "list.h"
#include "stdint.h"
#include "thread.h"

//信号量
struct semaphore {
    uint8_t  value;
    struct   list waiters;
};

//锁
struct lock {
    struct   task_struct* holder;	    //当前持有者
    struct   semaphore semaphore;	    
    uint32_t holder_repeat_nr;		    //锁的持有者可能重复申请
};

void sema_init(struct semaphore* psema, uint8_t value); 
void sema_down(struct semaphore* psema);
void sema_up(struct semaphore* psema);
void lock_init(struct lock* plock);
void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);
#endif
