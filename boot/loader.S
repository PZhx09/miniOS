%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR


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

   total_mem_bytes dd 0;用一个双字保存内存容量，单位是字节  他前面有64个段描述符  64*8=512=0x200  所以这里内存地址是0xb00  


   ;gdtr48位  低16位是段界限，高32位是gdt的起始地址
   gdt_ptr  dw  GDT_LIMIT 
	    dd  GDT_BASE

;loader_start的地址变成了0xb0a
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


    ;进入到保护模式之后，bios中断就不能用了，所以赶紧在进保护模式之前调用bios中断获取内存容量，为后面建立页表机制做铺垫
    ;获取内存容量，bios中断是INT 0X15  
    ;功能号0x88->最简单，但是最多检测到64MB内存
    ;功能号0xe801 能够检测到低15MB内存和16MB-4GB内存  32位平台下已经够用了
    ;功能号0xe820 能够检测到所有内存 Linux检测内存就是用这个
    ;虚拟机里我设置了32MB内存，理论上0x88就够用了，但是为了正规一些，使用e801足够满足32位平台最大内存4GB
get_mem_e801:
    mov ax,0xe801    ;ax放低15MB内存，单位1kb  bx放高于16mb的内存 单位64kb
    int 0X15      
    jc mem_get_error  ;如果中断失败，会置flags寄存器地CF位为1    

    ;1 先算出低15M的内存   
    ;实模式里当操作数为8位时，操作数与AL相乘并且存放在AX中，当操作数为16位时，操作数与AX相乘，高16位存放在DX中，低16位存放在AX中
    mov cx,0x400	     ;0x400=1024
    mul cx 
    shl edx,16           ;dx存放高位，下面将dx里的高位和ax里的低位拼在一起
    and eax,0x0000FFFF   ;清空 eax的高16位，
    add edx,eax
    add edx, 0x100000    ;历史原因e801会忽略掉memory hole 15-16mb那块
    mov esi,edx	     

    ;2 计算16MB以上的内存容量
    xor eax,eax
    mov ax,bx		
    mov ecx, 0x10000	;64kb=0x10000字节
    mul ecx		;32位乘法,默认的被乘数是eax,积为64位,高32位存入edx,低32位存入eax.
    add esi,eax
    mov edx,esi		

    mov [total_mem_bytes], edx




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
    ;三步完成，后面都是保护模式了   也就是32位平台   之后就没有办法是用BIOS中断了，因为bios中断都是16位平台下的汇编指令

    jmp  SELECTOR_CODE:p_mode_start   ;很重要的一条指令！！！！！！
    ;清空流水线并串行化处理器  CPU执行流水线 是取指、译码、执行、访存、写回   下面指令都是32位的，但是已经按照16位读取了   所以要清空流水线
    ;不过这个为什么5个nop不行？是因为nop16位和32位编码不同吗？  网上查的远跳转倒是可以。
mem_get_error:
    hlt


[bits 32]; nasm的伪指令，起了大作用   告诉编译请，下面的代码按照32位的来进行编译(16位实模式和32位保护模式的编码显然不一样)
p_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp,LOADER_BASE_ADDR   ;用loader开始的地址作为栈底，扩展方向不一样  这样很安全
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    mov byte [gs:320], 'P'
    mov byte [gs:321], 0x41 
    mov byte [gs:322], 'E'
    mov byte [gs:323], 0x41 

    ;加载内核，分页之前做这步好了，不然变成虚拟地址还挺麻烦的
    mov eax, KERNEL_START_SECTOR        
    mov ebx, KERNEL_BIN_BASE_ADDR       
    mov ecx, 200            ;内核先暂定200个扇区

    call rd_disk_32  ;之前mbr写的那个没办法直接调，重新在32位平台写一个

    ;1、创建页目录表和页表
    call setup_page
    
    ;开启了分页之后，Linux里是把3GB以上给OS   0-3GB是用户程序的
    ;所以对于原来的gdt来修改GDTR里面存的地址，而且对于属于内核的段(显卡段)要映射到3GB之上
    sgdt [gdt_ptr]       ; 存储到原来gdt所有的位置

    
    mov ebx, [gdt_ptr + 2]  
    or dword [ebx + 0x18 + 4], 0xc0000000      ;显卡段是第3个段

    
    add dword [gdt_ptr + 2], 0xc0000000 ;将整个GDT映射到3GB以上

    add esp, 0xc0000000        ; 将栈指针同样映射到内核地址

    ;2、把页目录地址赋给cr3
    mov eax, PAGE_DIR_TABLE_POS
    mov cr3, eax

    ; 3、打开cr0的pg位(第31位)
    mov eax, cr0
    or eax, 1<<31
    mov cr0, eax

    lgdt [gdt_ptr]             ;开启分页，重新加载gdt

    mov byte [gs:480], 'V'     
    mov byte [gs:481], 0x41 

    mov byte [gs:482], 'A'     
    mov byte [gs:483], 0x41 
    
    call kernel_init
    mov esp, 0xc009f000   ;根据内核地址选择
    jmp KERNEL_ENTRY_POINT                

    jmp $

;创建页目录表和页表  
setup_page:
;先把页目录占用的空间逐字节清0，页目录表占4KB
    mov ecx, 4096
    mov esi, 0
clear_page_dir:
    mov byte [PAGE_DIR_TABLE_POS + esi], 0
    inc esi
    loop clear_page_dir
;二级页表的好处之一就是没必要把全部的页表刚上来就创建出来，所以这里只创建重要的

create_pde:             ; 创建页目录项（PDE）  位置腾出来了，先创建3个  第1 个 第768个(上面是系统)  和第1024个
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x1000           ; 0x1000=4kb，这里eax是第一个页表的位置  
    mov ebx, eax            ;一会为第一个页表创建页表项


    or eax, PG_US_U | PG_RW_W | PG_P      ; 页目录项的属性RW和P位为1,US为1,表示用户属性,所有特权级别都可以访问.
    mov [PAGE_DIR_TABLE_POS + 0x0], eax       ; 第1个目录项,在页目录表中的第1个目录项写入第一个页表的位置(0x101000)及属性(7)
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax     ; 第768个页目录项，这之上是留给系统的
    ;这里看的elephant OS  把第1个页目录项和第768个页目录项都映射到第1个页表，这是为了把系统的内存映射到0xc0000000上做准备  这样的话前4m的内存，既是虚拟地址0x003fffff  又是0xc03fffff
    ;0指向前4M是为了之前loader中物理地址和线性地址一致，768指向前4M是为了把系统映射到0xc0000000上  
    sub eax, 0x1000
    mov [PAGE_DIR_TABLE_POS + 4092], eax      ; 使最后一个目录项指向页目录表自己的地址

;下面创建页表项(PTE)
    mov ecx, 256            ; 之前系统只占了1MB 也就是256个页
    mov esi, 0
    mov edx, PG_US_U | PG_RW_W | PG_P   
create_pte:            
    mov [ebx+esi*4],edx          
    add edx,4096
    inc esi
    loop create_pte;这里创建了第1个页表的256个页表项

;把内核769上面的页目录项给填了
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x2000         ; 此时eax为第二个页表的位置
    or eax, PG_US_U | PG_RW_W | PG_P 
    mov ebx, PAGE_DIR_TABLE_POS
    mov ecx, 254          ; 范围为第769~1022的所有目录项数量
    mov esi, 769
create_kernel_pde:
    mov [ebx+esi*4], eax
    inc esi
    add eax, 0x1000
    loop create_kernel_pde
    ret

;从磁盘eax读ecx个扇区到ebx
rd_disk_32:    
    mov esi,eax    
    mov di,cx      
    ;之后步骤与16位平台基本没有区别
    ;1、写入读取扇区数
    mov dx,0x1f2
    mov al,cl
    out dx,al            

    mov eax,esi   

      
    ;2、设置LBA地址
      
    mov dx,0x1f3                       
    out dx,al                          

      
    mov cl,8
    shr eax,cl
    mov dx,0x1f4
    out dx,al

    shr eax,cl
    mov dx,0x1f5
    out dx,al

    shr eax,cl
    and al,0x0f    
    or al,0xe0     ;device端口设置LBA寻址
    mov dx,0x1f6
    out dx,al

    ;3、command端口发送读命令
    mov dx,0x1f7
    mov al,0x20                        
    out dx,al

    ;4、cpu一直读取status寄存器检测是否够读取完成
not_ready:      
    nop
    in al,dx
    and al,0x88    ;第3位表示读取成功，第7位表示硬盘忙
    cmp al,0x08
    jnz not_ready     

    ;5、从data端口读取数据
    mov ax, di     

    mov dx, 256    
    mul dx
    mov cx, ax     ;cx存放读取次数
    mov dx, 0x1f0
go_on_read:
    in ax,dx    
    mov [ebx], ax
    add ebx, 2

    loop go_on_read
    ret




;elf文件格式，还要处理一下
;找到程序中的所有段，然后拷贝到应该在的地方
;参考elephantOS
kernel_init:
   xor eax, eax
   xor ebx, ebx   ;ebx记录程序头表地址
   xor ecx, ecx   ;cx记录程序头表中的program header数量
   xor edx, edx   ;dx 记录program header尺寸,即e_phentsize

   mov dx, [KERNEL_BIN_BASE_ADDR + 42]    ; 偏移文件开始部分28字节的地方是e_phoff移文件42字节处的属性是e_phentsize,表示program header大小
   mov ebx, [KERNEL_BIN_BASE_ADDR + 28]   ; 偏移文件开始部分28字节的地方是e_phoff,表示第1 个program header在文件中的偏移量

   add ebx, KERNEL_BIN_BASE_ADDR
   mov cx, [KERNEL_BIN_BASE_ADDR + 44]    ; 偏移文件开始部分44字节的地方是e_phnum,表示有几个program header
each_segment:
   cmp byte [ebx + 0], PT_NULL      ; 若p_type等于 PT_NULL,说明此program header未使用。
   je PTNULL

   ;为函数memcpy压入参数,参数是从右往左依然压入.函数原型类似于 memcpy(dst,src,size)
   push dword [ebx + 16]      ; program header中偏移16字节的地方是p_filesz,压入函数memcpy的第三个参数:size
   mov eax, [ebx + 4]       ; 距程序头偏移量为4字节的位置是p_offset
   add eax, KERNEL_BIN_BASE_ADDR    ; 加上kernel.bin被加载到的物理地址,eax为该段的物理地址
   push eax         ; 压入函数memcpy的第二个参数:源地址
   push dword [ebx + 8]       ; 压入函数memcpy的第一个参数:目的地址,偏移程序头8字节的位置是p_vaddr，这就是目的地址
   call mem_cpy         ; 调用mem_cpy完成段复制
   add esp,12         ; 清理栈中压入的三个参数
PTNULL:
   add ebx, edx         ; edx为program header大小,即e_phentsize,在此ebx指向下一个program header 
   loop each_segment
   ret

;----------  逐字节拷贝 mem_cpy(dst,src,size) ------------
;输入:栈中三个参数(dst,src,size)
;输出:无
;---------------------------------------------------------
mem_cpy:          
   cld
   push ebp
   mov ebp, esp
   push ecx      ; rep指令用到了ecx，但ecx对于外层段的循环还有用，故先入栈备份
   mov edi, [ebp + 8]    ; dst
   mov esi, [ebp + 12]     ; src
   mov ecx, [ebp + 16]     ; size
   rep movsb       ; 逐字节拷贝

   ;恢复环境
   pop ecx    
   pop ebp
   ret