;created by hx
;this is mbr
;先写个输出字符到屏幕上的程序测试MBR
;主要完成功能  清屏(清空bios输出的杂乱信息)  输出一个字符显示接替bios

%include "boot.inc"


SECTION MBR vstart=0x7C00
;利用CS里面的0清空段寄存器
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7C00 ;栈往低处走
    mov ax,0xB800
    mov gs,ax ;不能用立即数直接赋值段寄存器

;清屏(功能号06)-------------------------------------------------
    mov ax,0x0600
    mov bx,0x0700
    mov cx,0
    mov dx,0x184f ;VGA模式下80*25

    int 0x10

;利用显存直接在文本模式显示器上打印字符
    mov byte [gs:0x00],'H';低字节放字符
    mov byte [gs:0x01], 0x41  ;高字节放字符的元信息

    mov byte [gs:0x02],'e'
    mov byte [gs:0x03], 0x41  

    mov byte [gs:0x04],'l'
    mov byte [gs:0x05], 0x41  

    mov byte [gs:0x06],'l'
    mov byte [gs:0x07], 0x41  

    mov byte [gs:0x08],'o'
    mov byte [gs:0x09], 0x41  



    mov eax,LOADER_START_SECTOR ;lba地址
    mov bx,LOADER_BASE_ADDR   ;loader应该被加载的位置
    mov cx,4                ;loader仅占一个扇区
    call rd_disk

    jmp LOADER_BASE_ADDR
    
    rd_disk:

    mov esi,eax
    mov di,cx    ;备份

    ;1、写入sector count端口，设置读取扇区数
    mov dx,0x1F2
    mov al,cl
    out dx,al   ;一次写一个字

    mov eax,esi
    ;2、写入LBA low mid high device端口，设置LBA地址
    mov dx,0x1F3;low端口
    out dx,al

    shr eax,8;
    mov dx,0x1F4;mid端口
    out dx,al

    shr eax,8;
    mov dx,0x1F5;high端口
    out dx,al

    shr eax,8
    and al,0x0f;device端口 4位是lba地址，4位是控制相关的
    or al,0xe0;1110,表示采用lba寻址
    mov dx,0x1F6
    out dx,al

    ;3、扇区数，和位置已经确定，写入command端口，进行读取
    mov dx,0x1F7
    mov al,0x20
    out dx,al

    ;4、检查状态读取
not_ready:
    nop
    in al,dx
    and al,0x88
    cmp al,0x08
    jnz not_ready

    ;数据已经准备好了，从data端口读取
    mov ax,di
    mov dx,256
    mul dx
    mov cx,ax  ;一次读取一个字，一个扇区要读取256次
    mov dx,0x1F0
go_on_read:
    in ax,dx
    mov [bx],ax
    add bx,2
    loop go_on_read
    ret




    times 510-($-$$) db 0
    magic db 0x55,0xaa

