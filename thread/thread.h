#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H
#include "stdint.h"
#include "list.h"

// 自定义通用函数类型,它将在很多线程函数中做为形参类型
typedef void thread_func(void*); //void别名thread_func(void*)

//设置进线程状态
enum task_status {
   TASK_RUNNING,//0
   TASK_READY,  //1
   TASK_BLOCKED,//2
   TASK_WAITING,//3
   TASK_HANGING,//4
   TASK_DIED    //5
};

//intr_stack  在线程PCB所在页的顶端
//当线程被中断打断时，按照这个结果压入上下文(比对kernel.S中intr entry  顺序必须一直)
struct intr_stack {
    uint32_t vec_no;	 // 对应kernel.S push %1压入的中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;	 //依次压入EAX、ECX、EDX、EBX、ESP、EBP、ESI、EDI   虽然pushad把esp也压入,但esp是不断变化的,所以会被popad忽略
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

// 以下由cpu从低特权级进入高特权级时压入(CPU自动压入的)
    uint32_t err_code;		 // 有些中断不会自动压入错误码，但是在kernel中抹平了差异
    void (*eip) (void);
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

//线程自己的栈,用于存储线程中待执行的函数和用来switch_to时保存上下文环境

struct thread_stack {
	//ABI规则函数调用前后，ebp、ebx、edi、esi、esp不能被改变
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    //第一次运行时，指向待运行函数kernel_thread
    //switch_to回来时，指向返回地址
    void (*eip) (thread_func* func, void* func_arg);

    //只有线程第一次上CPU才会调用kernel_thread所以下面的属性，只有第一次有用
    void (*unused_retaddr);//kernel_thread是“有去无回”的，所以不应该用call而是用ret“调用”(把kernel_thead作为返回地址压栈，然后ret  这个占位假装是kernel_thread的返回地址)   这个只起到一个占位的作用
    thread_func* function;   //kernel_thread要运行的函数
    void* func_arg;    //kernel_thread运行函数的参数
};

/* 进程或线程的pcb,程序控制块 */
struct task_struct {
    uint32_t* self_kstack;	 // 各内核线程都用自己的内核栈
    enum task_status status;
    char name[16];
    uint8_t priority;     // 优先级
    uint8_t ticks;     //时间片，系统简单点，每次初始化时间片=线程优先级

    uint32_t elapsed_ticks;//总运行时间片


    struct list_elem general_tag;//线程在一般链表的节点，例如就绪队列、等待队列            


    struct list_elem all_list_tag;//线程在所有线程链表的节点

    uint32_t* pgdir;              //进程的页表，每个进程一套页表。如果是线程，这里为NULL
    uint32_t stack_magic;	 //魔数，用来检测边界  防止栈超出边界
};

void thread_create(struct task_struct* pthread, thread_func function, void* func_arg);
void init_thread(struct task_struct* pthread, char* name, int prio);
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg);
struct task_struct* running_thread(void);
void schedule(void);
void thread_init(void);
#endif
