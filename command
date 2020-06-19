编译汇编:
nasm -o xx.bin xx.s

nasm -f elf -o print.o print.S  -f指定目标文件的格式
编译C语言:   
gcc -c -o main.o main.c    -c表示只进行编译、汇编  不进行链接
gcc -I lib/kernel/ -c -o kernel/main.o kernel/main.c  -I指定要链接的库

链接:
ld main.o -Ttext 0xc0001500 -e main -o kernel.bin
ld kernel/main.o -Ttext 0xc0001500 -e main -o kernel/kernel.bin lib/kernel/print.o


写入硬盘
dd if=xxx of=xxx bs=512 count conv=notrunc


dd if=./loader.bin of=/home/pz/bochs/c.img bs=512 count=4 seek=2  conv=notrunc

dd if=kernel/bin of=/home/pz/bochs/c.img if=kernel/kernel.bin  bs=512 count=200 seek=9 conv=notrunc


bochs 常用指令
1、b 在物理地址上设置断断点
2、u/1 反汇编物理地址上的指令
3、c 继续执行
4、info 可以查看各种寄存器信息  例如 info CPU info gdt
5、单步执行 s
