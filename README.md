
# miniOS

参考Linux制作的一个简易OS内核。

## miniOS目前包含的部分


* MBR
* 实模式到保护模式的切换
* 内存分页
* 内存管理
* 中断
* 进程/线程
* 简易的键盘驱动

##miniOS尚未完成的部分
* 系统调用
* 硬盘驱动
* 文件系统
* shell界面


## 环境配置
预装依赖
```javascript
sudo apt-get install build-essential nasm
sudo apt-get install libgtk2.0-dev libxrandr-dev libx11-dev
sudo apt-get install bochs-x
```
bochs配置文件
```javascript
 #内存
megs: 32

#BIOS和VGABIOS的路径
    vgaromimage: file=/home/pz/bochs/share/bochs/VGABIOS-lgpl-latest  
    romimage: file=/home/pz/bochs/share/bochs/BIOS-bochs-latest 

#鼠标键盘
    mouse: enabled=0
    keyboard: keymap=/home/pz/bochs/share/bochs/keymaps/x11-pc-us.map

#硬盘信息
    boot: disk
    ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
    ata0-master: type=disk, path="/home/pz/bochs/c.img",mode=flat

    log: bochs.txt
```


## 参考书目
* 《Linux内核设计的艺术》
* 《深入理解Linux内核》
* 《x86汇编从实模式到保护模式》
* 《操作系统真相还原》
* 《一个64位操作系统的设计与实现》


