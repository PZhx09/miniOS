%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
jmp loader_start

   GDT_BASE:   dd    0x00000000 ;第一个段描述符必须是0  防止段寄存器未初始化就直接访问
	       dd    0x00000000

   CODE_DESC:  dd    0x0000FFFF 
	       dd    DESC_CODE_HIGH4

   DATA_STACK_DESC:  dd    0x0000FFFF
		     dd    DESC_DATA_HIGH4

   VIDEO_DESC: dd    0x80000007	       ;段基址是0XB8000正好对应文本模式在的内存地址
	       dd    DESC_VIDEO_HIGH4  

   GDT_SIZE   equ   $ - GDT_BASE      
   GDT_LIMIT   equ   GDT_SIZE -	1    ;gdt的段界限   规定的段界限就是gdt大小-1   这里感觉和段描述符段界限都很奇怪
   times 60 dq 0					 ; 预留一定区域放段描述符，后续添加
   SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0         ; cs段选择子
   SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0	 ; ds段选择子
   SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0	 ; gs段选择子

   
   ;gdtr48位  低16位是段界限，高32位是gdt的起始地址
   gdt_ptr  dw  GDT_LIMIT 
	    dd  GDT_BASE


   loader_start:

    mov byte [gs:160],'l';低字节放字符
    mov byte [gs:161], 0x41  ;高字节放字符的元信息

    mov byte [gs:162],'o'
    mov byte [gs:163], 0x41  

    mov byte [gs:164],'a'
    mov byte [gs:165], 0x41  

    mov byte [gs:166],'d'
    mov byte [gs:167], 0x41  

    mov byte [gs:168],'e'
    mov byte [gs:169], 0x41  

    mov byte [gs:170],'r'
    mov byte [gs:171], 0x41 


    ;loader准备进入保护模式---------------------------------------------------------------------------------

    ;1、打开A20  冲破地址回绕
    in al,0x92
    or al,0000_0010B
    out 0x92,al


    ;2、加载gdt
    lgdt [gdt_ptr]  ;内存直接寻址

    ;3、cr0的PE位置1   PE不就是protected enable的意思嘛
    mov eax,cr0
    or eax,1
    mov cr0,eax
    ;三步完成，后面都是保护模式了   也就是32位平台

    jmp  SELECTOR_CODE:p_mode_start   ;很重要的一条指令！！！！！！
    ;清空流水线并串行化处理器  CPU执行流水线 是取指、译码、执行、访存、写回   下面指令都是32位的，但是已经按照16位读取了   所以要清空流水线
    ;不过这个为什么5个nop不行？是因为nop16位和32位编码不同吗？  网上查的远跳转倒是可以。

[bits 32]; nasm的伪指令，起了大作用   告诉编译请，下面的代码按照32位的来进行编译(16位实模式和32位保护模式的编码显然不一样)
p_mode_start:
   mov ax, SELECTOR_DATA
   mov ds, ax
   mov es, ax
   mov ss, ax
   mov esp,LOADER_BASE_ADDR   ;用loader开始的地址作为栈底，扩展方向不一样  这样很安全
   mov ax, SELECTOR_VIDEO
   mov gs, ax

   mov byte [gs:480], 'P'
   mov byte [gs:481], 0x41 
   mov byte [gs:482], 'P'
   mov byte [gs:483], 0x41 

    jmp $