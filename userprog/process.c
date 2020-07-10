#include "process.h"
#include "global.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"    
#include "list.h"    
#include "tss.h"    
#include "interrupt.h"
#include "string.h"
#include "console.h"
#define PG_SIZE 4096
extern void intr_exit(void);//从高优先级返回低优先级，利用中断返回实现

//构建用户进程
void start_process(void* filename_) {//进程是程序的一次执行，用户进程肯定是从文件系统加载上来的程序(但是现在还没有实现文件系统，所以这里有些问题，是“假的”程序)
    void* function = filename_;//以后有了文件系统之后，肯定就不是这样写了
    struct task_struct* cur = running_thread();
    cur->self_kstack += sizeof(struct thread_stack);
    //构造中断栈
    struct intr_stack* proc_stack = (struct intr_stack*)cur->self_kstack;	 
    proc_stack->edi = proc_stack->esi = proc_stack->ebp = proc_stack->esp_dummy = 0;
    proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax = 0;
    proc_stack->gs = 0;		 
    proc_stack->ds = proc_stack->es = proc_stack->fs = SELECTOR_U_DATA;
    proc_stack->eip = function;	 // 待执行的用户程序地址
    proc_stack->cs = SELECTOR_U_CODE;//用户态的选择子
    proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);//设置好eflags
    proc_stack->esp = (void*)((uint32_t)get_a_page(PF_USER, USER_STACK3_VADDR) + PG_SIZE) ;//用户栈
    proc_stack->ss = SELECTOR_U_DATA; //用户栈选择子
    asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (proc_stack) : "memory");//利用中断返回实现特权级的变化
}

//激活页表
void page_dir_activate(struct task_struct* p_thread) {
   

    //若为内核线程,需要重新填充页表为0x100000(之前内核页表放在了这里)
    uint32_t pagedir_phy_addr = 0x100000;  
    if (p_thread->pgdir != NULL)	
    {   // 用户态进程有自己的页目录表
        pagedir_phy_addr = addr_v2p((uint32_t)p_thread->pgdir);
    }

    //装载新页表
    asm volatile ("movl %0, %%cr3" : : "r" (pagedir_phy_addr) : "memory");
}

//激活线程或进程的页表,更新tss中的esp0为进程的特权级0的栈
void process_activate(struct task_struct* p_thread) {
    ASSERT(p_thread != NULL);
     
    //激活页表
    page_dir_activate(p_thread);

    //内核线程特权级本身就是0,中断并不涉及到特权级切换，CPU不会去TSS拿esp,故不需要更新esp0
    //这里做的有些问题，因为进程可能多线程，等等看看别人是怎么解决多线程进程调度的问题的
    if (p_thread->pgdir) 
    {
        //更新该进程的esp0,用于此进程被中断时保留上下文
        update_tss_esp(p_thread);
    }
}


//创建页目录表
uint32_t* create_page_dir(void) {

    uint32_t* page_dir_vaddr = get_kernel_pages(1);//页表都是放在内核空间
    if (page_dir_vaddr == NULL) {
        console_put_str("create_page_dir: get_kernel_page failed!");
        return NULL;
    }


    //1、复制系统所在1G空间的页表  0x700=768
    memcpy((uint32_t*)((uint32_t)page_dir_vaddr + 0x300*4), (uint32_t*)(0xfffff000+0x300*4), 1024);

    //2、更新页目录表地址
    uint32_t new_page_dir_phy_addr = addr_v2p((uint32_t)page_dir_vaddr);
    //页目录地址是存入在页目录的最后一项,更新页目录地址为新页目录的物理地址
    page_dir_vaddr[1023] = new_page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1;
    return page_dir_vaddr;
}

//之前只有内核虚拟内存池，现在建立用户虚拟内池  创建用户虚拟地址位图
void create_user_vaddr_bitmap(struct task_struct* user_prog) {
    user_prog->userprog_vaddr.vaddr_start = USER_VADDR_START;
    uint32_t bitmap_pg_cnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START) / PG_SIZE / 8 , PG_SIZE);
    user_prog->userprog_vaddr.vaddr_bitmap.bits = get_kernel_pages(bitmap_pg_cnt);
    user_prog->userprog_vaddr.vaddr_bitmap.btmp_bytes_len = (0xc0000000 - USER_VADDR_START) / PG_SIZE / 8;
    bitmap_init(&user_prog->userprog_vaddr.vaddr_bitmap);
}

///创建用户进程
void process_execute(void* filename, char* name) { 

    struct task_struct* thread = get_kernel_pages(1);//task_struct肯定是在内核空间的
    init_thread(thread, name, default_prio); 
    create_user_vaddr_bitmap(thread);
    thread_create(thread, start_process, filename);//利用线程机制，执行start_process，从而构建用户进程
    thread->pgdir = create_page_dir();//创建页表
   
    enum intr_status old_status = intr_disable();//有关调度的一定要关中断
    ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    list_append(&thread_ready_list, &thread->general_tag);

    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
    list_append(&thread_all_list, &thread->all_list_tag);
    intr_set_status(old_status);
}

