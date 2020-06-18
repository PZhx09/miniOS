编译汇编:
nasm -o xx.bin xx.s

编译C语言:   
gcc -c -o main.o main.c    -c表示只进行编译、汇编  不进行链接

链接:
ld main.o -Ttext 0xc0001500 -e main -o kernel.bin



写入硬盘
dd if=xxx of=xxx bs=512 count conv=notrunc


dd if=./loader.bin of=/home/pz/bochs/c.img bs=512 count=4 seek=2  conv=notrunc



bochs 常用指令
1、b 在物理地址上设置断断点
2、u/1 反汇编物理地址上的指令
3、c 继续执行
4、info 可以查看各种寄存器信息  例如 info CPU info gdt
5、单步执行 s
