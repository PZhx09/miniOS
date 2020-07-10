#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "io.h"
#include "print.h"

#define PIC_M_CTRL 0x20	       // 8259A主片控制端口 0x20
#define PIC_M_DATA 0x21	       // 主片数据端口0x21
#define PIC_S_CTRL 0xa0	       // 从片控制端口0xa0
#define PIC_S_DATA 0xa1	       // 从片数据端口0xa1

#define IDT_DESC_CNT 0x30	 // 目前总共支持的中断数   0-0x20

#define EFLAGS_IF   1<<9       // eflags寄存器中的if位
#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl; popl %0" : "=g" (EFLAG_VAR)) //把eflag压入栈，再弹出到EFLAG_VAR变量



//中断门描述符   8字节  C语言中定义结构体，地址从低到高
struct gate_desc {
    uint16_t    func_offset_low_word;  //低字节0-15段内偏移
    uint16_t    selector;//低字节16-31，中断处理程序代码段选择子
    uint8_t     dcount;   //高字节0-7 无用
    uint8_t     attribute; //type、S、DPL、P
    uint16_t    func_offset_high_word; //高字节16-31  段内偏移
};


static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);
static struct gate_desc idt[IDT_DESC_CNT];   

extern intr_handler intr_entry_table[IDT_DESC_CNT];	    // 中断处理函数的入口数组 intr_handler就是void*函数指针

char* intr_name[IDT_DESC_CNT];           // 用于保存异常的名字
intr_handler idt_table[IDT_DESC_CNT];       // 中断处理程序数组  中断入口真正调用的函数

//初始化8259A   https://blog.csdn.net/longintchar/article/details/79439466   这里有参数意义
static void pic_init(void) {

    //初始化主片 注意ICW1-ICW4顺序不能变
    outb (PIC_M_CTRL, 0x11);   // 在 Linux-0.11 内核中，ICW1 被设置为 0x11。表示中断请求是边沿触发、多片 8259A 级联并且需要发送 ICW4。
    outb (PIC_M_DATA, 0x20);   // Linux-0.11 系统把主片的 ICW2 设置为 0x20，表示主片中断请求0~7级对应的中断号是 0x20~0x27
    outb (PIC_M_DATA, 0x04);   //  Linux-0.11 内核把8259A主片的 ICW3 设置为 0x04，即 S2=1，其余各位为0。也就是说IR2接从片
    outb (PIC_M_DATA, 0x01);   // Linux-0.11内核送往8259A主芯片和从芯片的 ICW4 命令字的值均为 0x01。表示 8259A 芯片被设置成普通全嵌套、非缓冲、非自动结束中断方式，并且用于 8086 及其兼容系统。

    //初始化从片
    outb (PIC_S_CTRL, 0x11);	// 同上
    outb (PIC_S_DATA, 0x28);	// 从片的 ICW2 设置成 0x28，表示从片中断请求8~15级对应的中断号是 0x28~0x2f
    outb (PIC_S_DATA, 0x02);	// 从芯片的 ICW3 被设置为 0x02，即其标识号为2，表示此从片连接到主片的IR2引脚。
    outb (PIC_S_DATA, 0x01);	// 同上

   
    //outb (PIC_M_DATA, 0xfd);//测试键盘
    outb (PIC_M_DATA, 0xfe);//主片只不屏蔽IR0  也就是时钟中断
    outb (PIC_S_DATA, 0xff);//屏蔽从片所有中断

    put_str("8259A_init done\n");
}

//构造中断门描述符
static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function) { 
    p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000FFFF;//入口地址低16位
    p_gdesc->selector = SELECTOR_K_CODE;//选择子
    p_gdesc->dcount = 0;//无意义
    p_gdesc->attribute = attr;//属性，这里是先定义好了
    p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >> 16;
}

//创建IDT
static void idt_desc_init(void) {
    int i;
    for (i = 0; i < IDT_DESC_CNT; i++) {
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]); 
    }
    put_str("   idt_desc_init done\n");
}


//写个通用的中断处理函数好了
static void general_intr_handler(uint8_t vec_nr) {
    if (vec_nr == 0x27 || vec_nr == 0x2f) {   //IRQ7和IRQ15会产生伪中断(spurious interrupt),无须处理。
        return;     
    }
    //准备打印异常信息  VGA文本模式80*25
    set_cursor(0);
    int cursor_pos = 0;
    while(cursor_pos < 320) {//清空一段区域
        put_char(' ');
        cursor_pos++;
    }

    set_cursor(0);    
    put_str("!!!!!!!      excetion message begin  !!!!!!!!\n");
    set_cursor(88); 
    put_str(intr_name[vec_nr]);
    if (vec_nr == 14) {    // 若为Pagefault,将缺失的地址打印出来并悬停
        int page_fault_vaddr = 0; 
        asm ("movl %%cr2, %0" : "=r" (page_fault_vaddr));   // cr2是存放造成page_fault的地址
        put_str("\npage fault addr is ");put_int(page_fault_vaddr); 
    }
    put_str("\n!!!!!!!      excetion message end    !!!!!!!!\n");

    while(1);//真正的死循环，因为中断处理程序肯定是关中断
}

//注册0-19 这些异常的中断处理函数 
static void exception_init(void) {            
    int i;
    for (i = 0; i < IDT_DESC_CNT; i++) {
        // 默认为general_intr_handler，后面用到的中断单独再注册
        idt_table[i] = general_intr_handler;         
        intr_name[i] = "unknown";              
    }
    intr_name[0] = "#DE Divide Error";
    intr_name[1] = "#DB Debug Exception";
    intr_name[2] = "NMI Interrupt";
    intr_name[3] = "#BP Breakpoint Exception";
    intr_name[4] = "#OF Overflow Exception";
    intr_name[5] = "#BR BOUND Range Exceeded Exception";
    intr_name[6] = "#UD Invalid Opcode Exception";
    intr_name[7] = "#NM Device Not Available Exception";
    intr_name[8] = "#DF Double Fault Exception";
    intr_name[9] = "Coprocessor Segment Overrun";
    intr_name[10] = "#TS Invalid TSS Exception";
    intr_name[11] = "#NP Segment Not Present";
    intr_name[12] = "#SS Stack Fault Exception";
    intr_name[13] = "#GP General Protection Exception";
    intr_name[14] = "#PF Page-Fault Exception";
    // intr_name[15] 第15项是intel reversed，未使用
    intr_name[16] = "#MF x87 FPU Floating-Point Error";
    intr_name[17] = "#AC Alignment Check Exception";
    intr_name[18] = "#MC Machine-Check Exception";
    intr_name[19] = "#XM SIMD Floating-Point Exception";

}


//开中断并返回旧状态
enum intr_status intr_enable() {
    enum intr_status old_status;
    if (intr_get_status()==INTR_ON) {
   	    //原本就是开中断
        old_status = INTR_ON;
        return old_status;
    } else {
        old_status = INTR_OFF;
        asm volatile("sti");	 // 开中断,sti指令将IF位置1
        return old_status;
    }
}

//关中断,并且返回旧状态 
enum intr_status intr_disable() {     
    enum intr_status old_status;
    if (intr_get_status()==INTR_ON) {
        old_status = INTR_ON;
        asm volatile("cli" : : : "memory"); // 破坏描述部分，https://blog.csdn.net/szhlcy/article/details/102560866
        return old_status;
    } else {
        old_status = INTR_OFF;
        return old_status;
    }
}

//将中断状态设置为status 
enum intr_status intr_set_status(enum intr_status status) {
	if(status&INTR_ON)
	{
		return intr_enable();
	}
	else
	{
		return intr_disable();
	}
}

//获取当前中断状态
enum intr_status intr_get_status() {
    uint32_t eflags = 0; 
    GET_EFLAGS(eflags);
    if(EFLAGS_IF & eflags)
   	    return  INTR_ON;
    else
   	    return INTR_OFF;
}

//注册中断处理函数
void register_handler(uint8_t vector_no, intr_handler function) {

   idt_table[vector_no] = function; 
}

//完成有关中断的所有初始化工作
void idt_init() {
    put_str("idt_init start\n");
    idt_desc_init();	   // 初始化中断描述符表
    exception_init();    // 异常名初始化并注册通常的中断处理函数
    pic_init();		   // 初始化8259A

    //加载IDT  IDTR是一个48位的寄存器   拿64位的数据代替 0-16是表界限    16-48是中断描述符表的入口地址
    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));//指针4字节不能直接转64位，先转32再转64
    asm volatile("lidt %0" : : "m" (idt_operand));
    put_str("idt_init done\n");
}

