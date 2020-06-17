编译汇编:
nasm -o xx.bin xx.s

写入硬盘
dd if=xxx of=xxx bs=512 count conv=notrunc



bochs 常用指令
1、b 在物理地址上设置断断点
2、u/1 反汇编物理地址上的指令
3、c 继续执行
4、info 可以查看各种寄存器信息  例如 info CPU info gdt
5、单步执行 s
