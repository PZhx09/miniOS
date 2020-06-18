LOADER_BASE_ADDR equ 0x900   ;loader被加载内存位置
LOADER_START_SECTOR equ 0x2   ;loader写入了硬盘的0盘0道2扇区

PAGE_DIR_TABLE_POS equ 0x100000   ;页表被扔在了0x100000
KERNEL_BIN_BASE_ADDR equ 0x70000      ;kernel被加载进的地方
KERNEL_START_SECTOR equ 0x9           ;kernel的开始扇区
KERNEL_ENTRY_POINT equ 0xc0001500     ;

;构造段描述符
DESC_G_4K equ 1<<23  ;G设置1
DESC_D_32 equ 1<<22  ;D设置1  表示32位
DESC_L equ 0<<21     ;64位代码标记
DESC_AVL equ 0<<20   ;无用
DESC_LIMIT_CODE2  equ 1111b<<16      ;代码段段界限    这是高字节的那块段界限
DESC_LIMIT_DATA2  equ DESC_LIMIT_CODE2 ;数据段段界限
DESC_LIMIT_VIDEO2  equ 0 ;显卡段段界限只有0xbffff-0xb8000 =0x7fff   粒度是4k  这里显卡段有32k，按照正常来说应该是8才对，但是发现越界  查了一下公式是（描述符中段界限＋1) * （段界限的粒度大小4k或者 1) -1   也就是说8-1
DESC_P	    equ		  1<<15
DESC_DPL_0  equ		   00b<<13
DESC_DPL_1  equ		   01b<<13
DESC_DPL_2  equ		   10<<13
DESC_DPL_3  equ		   11<<13

DESC_S_CODE equ		     1<<12
DESC_S_DATA equ	  DESC_S_CODE
DESC_S_sys  equ		     0_000000000000b

DESC_TYPE_CODE  equ	      1000b<<8	;X C R A     x=1表示是代码段，R=1表示可读，C=0 表示非一致性代码段(这个和跳转时的特权级有关)  A accessed 显然没有访问过  
DESC_TYPE_DATA  equ	      0010b<<8	;X E W A     X=0表示是数据段  E=0表示向上扩展  W=1 表示可写 A accessed 显然没有访问过  


;将上面数据组合成三个段描述符的高字节
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b


;构造选择子   选择子放在段寄存器上，也就是说有16位    0-1位表示RPL特权级  2位表示TI  就是用gdt还是ldt  剩下13位是段描述符的索引 
RPL0  equ   00b
RPL1  equ   01b
RPL2  equ   10b
RPL3  equ   11b
TI_GDT	 equ   000b
TI_LDT	 equ   100b

;页表的字段
PG_P  equ   1b
PG_RW_R	 equ  00b ;可读不可写
PG_RW_W	 equ  10b  ;可读可写
PG_US_S	 equ  000b ;us为0表示系统页表
PG_US_U	 equ  100b ;us为1表示用户程序


;
PT_NULL equ 0
