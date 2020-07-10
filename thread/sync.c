#include "sync.h"
#include "list.h"
#include "global.h"
#include "debug.h"
#include "interrupt.h"

//信号量初始化
void sema_init(struct semaphore* psema, uint8_t value) {
    psema->value = value;       
    list_init(&psema->waiters); //初始化信号量的等待队列
}

//初始化锁
void lock_init(struct lock* plock) {
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    sema_init(&plock->semaphore, 1);  // 信号量初值为1
}

//获得信号量
void sema_down(struct semaphore* psema) {
    //这里必须关中断
    enum intr_status old_status = intr_disable();
    while(psema->value == 0) {	// 若value为0,表示已经被别人持有  注意这里必须是while  if有问题，因为唤醒后，可能仍无法获取信号量
        ASSERT(!elem_find(&psema->waiters, &running_thread()->general_tag));
        
        if (elem_find(&psema->waiters, &running_thread()->general_tag)) {
	         PANIC("sema_down: thread blocked has been in waiters_list\n");
        }
        //无法获得信号量，将自己加入到该信号量的等待队列中
        list_append(&psema->waiters, &running_thread()->general_tag); 
        thread_block(TASK_BLOCKED);    
    }
    //value不为0，或者被唤醒   获取信号量
    psema->value--;
    ASSERT(psema->value == 0);	    
    intr_set_status(old_status);
}

//释放信号量
void sema_up(struct semaphore* psema) {
    enum intr_status old_status = intr_disable();
    ASSERT(psema->value == 0);	    
    if (!list_empty(&psema->waiters)) {
        struct task_struct* thread_blocked = elem2entry(struct task_struct, general_tag, list_pop(&psema->waiters));
        thread_unblock(thread_blocked);
    }
    psema->value++;
    ASSERT(psema->value == 1);	    

    intr_set_status(old_status);
}

//获取锁
void lock_acquire(struct lock* plock) {

    if (plock->holder != running_thread()) { 
        sema_down(&plock->semaphore);    //对信号量P操作,原子操作
        plock->holder = running_thread();
        ASSERT(plock->holder_repeat_nr == 0);
        plock->holder_repeat_nr = 1;
    } else {
        plock->holder_repeat_nr++;
    }
}

//释放锁
void lock_release(struct lock* plock) {
    ASSERT(plock->holder == running_thread());
    if (plock->holder_repeat_nr > 1) {
        plock->holder_repeat_nr--;
        return;
    }
    ASSERT(plock->holder_repeat_nr == 1);

    plock->holder = NULL;	   //锁的持有者设为NULL
    plock->holder_repeat_nr = 0;
    sema_up(&plock->semaphore);	   //释放信号量
}

