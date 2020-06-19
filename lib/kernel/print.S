TI_GDT equ  0
RPL0  equ   0
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

section .data
buffer_for_put_int    dq    0 
[bits 32]
section .text
;编写函数put_char---------------------------------------------------------------------------
;像屏幕光标处输出一个字符   要判定是不是特殊的字符 
;具体参考https://www.jianshu.com/p/7ace5e88704b
;函数调用规约遵循cdecl，入栈方式从右向左，由调用者清理栈

global put_str
put_str:
;由于本函数中只用到了ebx和ecx,只备份这两个寄存器
    pushad
    xor ecx, ecx            ; 准备用ecx存储参数,清空
    mov ebx, [esp + 36]         
not_end:
    mov cl, [ebx]
    cmp cl, 0            ; char*字符串以\0结尾
    jz str_over
    push ecx          
    call put_char
    add esp, 4           ; cdecl规约，回收参数所占的栈空间
    inc ebx           ; 使ebx指向下一个字符
    jmp not_end
str_over:
    popad
    ret


global put_char;汇编语言要声明global才能在C语言中被看到
put_char:
    pushad	   ;push all double 保护现场
    mov ax, SELECTOR_VIDEO	       ; 保险起见构造一个显卡段的段选择子
    mov gs, ax

    ;先获取光标位置  注意，这里不能够用bios中断了
    mov dx, 0x03d4  ;索引寄存器
    mov al, 0x0e	   ;cursor location high register
    out dx, al
    mov dx, 0x03d5  ;0x3d5数据寄存器
    in al, dx	   
    mov ah, al

    ;同样方式获得低8位
    mov dx, 0x03d4
    mov al, 0x0f  ;cursor location low register
    out dx, al
    mov dx, 0x03d5 
    in al, dx

    ;将光标存入bx
    mov bx, ax	  
    
    mov ecx, [esp + 36]	      ;pushad 4*8=32 返回地址4  所以这里exp36是要打印赐福
    cmp cl, 0xd				  ;carriage return   回车
    jz is_carriage_return
    cmp cl, 0xa            ;LF (NL line feed, new line)  换行
    jz  is_line_feed

    cmp cl, 0x8				  ;Bbackspace 退格
    jz is_backspace
    jmp  put_other	   


is_backspace:		      

    dec bx
    shl bx,1       ;一个字符两个字节，所以这里乘2
    mov byte [gs:bx], 0x20		  ;补上空格
    inc bx
    mov byte [gs:bx], 0x07
    shr bx,1
    jmp set_cursor


put_other:
    shl bx, 1				  ; 一个字符两个字节，所以这里乘2
    mov [gs:bx], cl			  
    inc bx
    mov byte [gs:bx],0x07		  
    shr bx, 1				  
    inc bx				  ; 下一个光标值
    cmp bx, 2000		   
    jl set_cursor			  ; jl  小于跳转   2000的时候就该换行了

is_line_feed:				  ;查了下 回车和换行不是一个操作，回车是到行首位置，换行是到下一行  这里就按照linux回车换行处理好了
is_carriage_return:		  ;\n是换行，\r是回车  这里当成一个处理
					            ;光标移到行首，然后加一行
                          ; div指令:如果除数是8位，被除数就是16位，位于ax中。所得的结果商在寄存器al中，余数在寄存器ah中。
    mov ax, bx				  ; ax是被除数.
    mov cl, 80				  
    div cl				    
    mov al,ah
    and ax,0x00ff
    sub bx, ax			  ;   sub bx, ah    减去余数，就相当于取整了 这里逻辑上没问题，编译不通过，16位数-8位数，错了

is_carriage_return_end:		  ; 回车符CR处理结束
    add bx, 80
    cmp bx, 2000
is_line_feed_end:			  ; 若是LF(\n),将光标移+80便可。  
    jl set_cursor

;光标位置大于灯鱼2000 需要滚屏！！！！
roll_screen:				  
    cld                      ;https://blog.csdn.net/zhenyongyuan123/article/details/8364011  方向标志还是需要设置的
    mov ecx, 960				  ; 一共有2000-80=1920个字符要搬运,共1920*2=3840字节.一次搬两个字,共3840/4=960次 
    mov esi, 0xb80a0			  
    mov edi, 0xb8000			  
    rep movsd				  

    ;将最后一行填充为空白
    mov ebx, 3840			  ;  1920 * 2=240
    mov ecx, 80			
cls:
    mov word [gs:ebx], 0x0720		  ;0x0720是黑底白字的空格键
    add ebx, 2
    loop cls 
    mov bx,1920				  ;将光标值重置为1920,最后一行的首字符.


    ;打印完了，最后设置光标位置
set_cursor:   

    ;和获取差不多，先设置高8位，然后设置低8位
    mov dx, 0x03d4			 
    mov al, 0x0e				  
    out dx, al
    mov dx, 0x03d5			  
    mov al, bh       ;高8位
    out dx, al


    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5 
    mov al, bl      ;低8位
    out dx, al

end: 
    popad
    ret

;打印16进制整数
;这里注意x86是小端字节序(高字节在高地址，低字节在低地址)，应该自己倒一下
;32位数字，一共是4字节，8个16进制的数字
;挨个处理每4位，然后放到缓冲区
global put_int
put_int:
   pushad
   mov ebp, esp
   mov eax, [ebp+36]             
   mov edx, eax          ;edx存放要处理的32位数  小端字节序
   mov edi, 7                          ; 指定在buffer_for_put_int中初始的偏移量
   mov ecx, 8               ; 32位数字中,16进制数字的位数是8个
   mov ebx, buffer_for_put_int

;每次处理4位
bits_4:                
   and edx, 0x0000000F            
   cmp edx, 9               ; 数字0～9和a~f需要分别处理成对应的字符
   jg is_A2F                ;jg大于转移
   add edx, '0'                
   jmp store
is_A2F:
   sub edx, 10              ; 10=A 11=B 以此类推
   add edx, 'A'

;存储的时候倒过来，高字节在低地址，低位在高地址
store:
; 此时dl中应该是数字对应的字符的ascii码
   mov [ebx+edi], dl           
   dec edi
   shr eax, 4
   mov edx, eax 
   loop bits_4


ready:;要先找到第一个不为0的数
   inc edi               ;这时候[ebx+edi]是指向缓冲区的第一个字节
skip_prefix_0:  
   cmp edi,8                ; 若已经比较第9个字符了，表示待打印的字符串为全0 
   je full0 

go_on_skip:   
   mov cl, [buffer_for_put_int+edi]
   inc edi
   cmp cl, '0' 
   je skip_prefix_0           
   dec edi                     
   jmp put_each_num

full0:
   mov cl,'0'               ; 输入的数字为全0时，则只打印0
put_each_num:
   push ecx              
   call put_char
   add esp, 4
   inc edi               ; 索引+1
   mov cl, [buffer_for_put_int+edi]         ; 获取下一个字符到cl寄存器
   cmp edi,8
   jl put_each_num
   popad
   ret
