;先写个输出字符到屏幕上的程序测试MBR
;主要完成功能  清屏(清空bios输出的杂乱信息)  光标处输出想要的字符(先获取光标，然后输出字符到屏幕上)

SECTION MBR vstart=0x7C00
;利用CS里面的0清空段寄存器
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7C00 ;栈往低处走

;清屏(功能号06)-------------------------------------------------
    mov ax,0x0600
    mov bx,0x0700
    mov cx,0
    mov dx,0x184f ;VGA模式下80*25

    int 0x10


;获取光标位置(功能号03,行在dh，列在dx)------------------------------------------
    mov ax,0x0300
    mov bh,0

    int 0x10


;打印字符串
    mov ax,hello
    mov bp,ax
    mov ax,0x1301
    mov bx,0x0007 
    mov cx,0xc

    int 0x10

    jmp $

    hello db "Hello World!"
    
    times 510-($-$$) db 0
    magic db 0x55,0xaa

