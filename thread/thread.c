#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "debug.h"
#include "interrupt.h"
#include "print.h"
#include "memory.h"

#define PG_SIZE 4096


struct task_struct* main_thread;    //主线程PCB
struct list thread_ready_list;      //就绪队列
struct list thread_all_list;        //所有任务队列
static struct list_elem* thread_tag;//用于保存队列中的线程结点

extern void switch_to(struct task_struct* cur, struct task_struct* next);//切换函数

//获取当前PCB  与Linux2.6一样
struct task_struct* running_thread() {
    uint32_t esp; 
    asm ("mov %%esp, %0" : "=g" (esp));//:输出:输入:限制符

    return (struct task_struct*)(esp & 0xfffff000);//栈和task_struct在同一页，
}

//线程执行kernel_thread ,kernel_thread执行实际的函数
static void kernel_thread(thread_func* function, void* func_arg) {
    intr_enable();//必须保证开中断，不然的话时间片就不动了，无法正常调度
    function(func_arg); 
}

//初始化线程栈thread_stack,将待执行的函数和参数放到thread_stack中相应的位置
void thread_create(struct task_struct* pthread, thread_func function, void* func_arg) {
    //栈的最低端是中断栈  然后是线程栈

    pthread->self_kstack -= sizeof(struct intr_stack);//预留中断栈
    pthread->self_kstack -= sizeof(struct thread_stack);//预留线程栈

    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi = kthread_stack->edi = 0;
}

//初始化线程
void init_thread(struct task_struct* pthread, char* name, int prio) {
    memset(pthread, 0, sizeof(*pthread));
    strcpy(pthread->name, name);
    if (pthread == main_thread) {
        //这里是把之前的main给包装成一个线程
        pthread->status = TASK_RUNNING;
    } else {
        pthread->status = TASK_READY;
    }

    pthread->priority = prio;
    pthread->ticks = prio;
    pthread->elapsed_ticks = 0;
    pthread->pgdir = NULL;
    //和Linux 2.6一样，task_struct所在页的高地址作为栈底   线程在内核态上的任何栈操作都在这里
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE);
    pthread->stack_magic = 0x19960903;	  
}

//创建一个线程，名字name，优先级pro，执行function，参数funca_arg
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg) {
 
    struct task_struct* thread = get_kernel_pages(1);//线程的task_struct显然在内核内存空间中

    init_thread(thread, name, prio);
    thread_create(thread, function, func_arg); 

    //asm volatile ("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret" : : "g" (thread->self_kstack) : "memory");
    //内联汇编，作用修改esp到线程的内核栈  ，初始化ebp、ebx、edi、esi  然后ret去执行kernel_thread

    ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));//加入就绪队列
    list_append(&thread_ready_list, &thread->general_tag);


    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));//加入全部线程队列
    list_append(&thread_all_list, &thread->all_list_tag);
    return thread;
}

//先有main，后有线程 所以这里要把main给做成线程
static void make_main_thread(void) {
    //main线程栈顶mov esp,0xc009f000，预留的task_struct也就是0xc009e000
    main_thread = running_thread();
    init_thread(main_thread, "main", 31);

    ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
    list_append(&thread_all_list, &main_thread->all_list_tag);
}

//任务调度
void schedule() {
    ASSERT(intr_get_status() == INTR_OFF);

    struct task_struct* cur = running_thread(); 
    if (cur->status == TASK_RUNNING) { //时间片用光，将其加入就绪队列
        ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
        list_append(&thread_ready_list, &cur->general_tag);
        cur->ticks = cur->priority;     //重置时间片
        cur->status = TASK_READY;
    } else { 
        //因为获取不了资源等原因换下CPU
    }

    ASSERT(!list_empty(&thread_ready_list));
    
    thread_tag = NULL;     
    thread_tag = list_pop(&thread_ready_list);   
    struct task_struct* next = elem2entry(struct task_struct, general_tag, thread_tag);
    next->status = TASK_RUNNING;
    switch_to(cur, next);
}

//初始化线程环境(其实就是初始化两个双向链表，和为main构造线程)
void thread_init(void) {
    put_str("thread_init start\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    
    make_main_thread();
    put_str("thread_init done\n");
}
