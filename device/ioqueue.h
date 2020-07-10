#ifndef __DEVICE_IOQUEUE_H
#define __DEVICE_IOQUEUE_H
#include "stdint.h"
#include "thread.h"
#include "sync.h"

#define bufsize 64

//环形缓冲区
struct ioqueue {
    //生产者消费者问题
    struct lock lock;//需要用锁来保证原子性
   
    struct task_struct* producer;//生产者，缓冲区没满，就往里面放数据   否则记录哪个生产者线程在此睡眠
    struct task_struct* consumer;//消费者，缓冲区不空时就继续从往里面拿数据,否则就睡眠,此项记录哪个消费者在此缓冲区上睡眠

    char buf[bufsize];			    // 缓冲区大小，可以放64个字符
    int32_t head;			    // 队首序号,数据往队首处写入
    int32_t tail;			    // 队尾序号,数据从队尾处读出
};

void ioqueue_init(struct ioqueue* ioq);
bool ioq_full(struct ioqueue* ioq);
char ioq_getchar(struct ioqueue* ioq);
void ioq_putchar(struct ioqueue* ioq, char byte);
#endif
