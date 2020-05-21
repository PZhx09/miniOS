# 开发环境配置指南

## 第一部分 配置虚拟机

## 第0步：虚拟机管理器

Vitrualbox 6.0.10

## 第1步：虚拟机镜像

Ubuntu 18.04.2 LTS

下载链接: https://ubuntu.com/download/desktop/thank-you?version=18.04.2&amp;architecture=amd64

## 第2步：预装依赖

| sudo apt-get install build-essential nasmsudo apt-get install libgtk2.0-dev ~~libxrandr-dev libx11-dev~~ sudo apt-get install bochs-x |
| --- |

## 第二部分 配置bochs

## 第1步：安装bochs

第一，获取bochs（最新稳定版）

| 下载链接: [https://sourceforge.net/projects/bochs/files/bochs/2.6.9/bochs-2.6.9.tar.gz/download](https://sourceforge.net/projects/bochs/files/bochs/2.6.9/bochs-2.6.9.tar.gz/download)tar -zxvf bochs-2.6.9.tar.gzcd bochs-2.6.9/ |
| --- |

第二，执行configure操作，如下所示：

| ./configure --prefix=/home/user/bochs --enable-debugger --enable-disasm --enable-iodebug --enable-x86-debugger --with-x --with-x11 libs=&#39;-lx11&#39;
说明：1)--prefix：用户自定义的bochs的安装路径2)--enable-xxxx：本环境所需的bochs的特性3)libs：本环境下bochs所需的系统库 |
| --- |

第三，构建和安装，如下所示：

| makemake install |
| --- |

## 第2步：配置bochs

第一，创建启动盘，对应文件为hd60M.img，如下所示：

| cd ~/bochs bin/bximage1：创建虚拟硬盘 → hd：硬盘 → Flat→60(这个是硬盘容量) |
| --- |

第二，设置bochs的配置文件，如下：

首先，将~/bochs/share/doc/boch/bochsrc-sample.txt复制~/bochs/bochsrc.txt；

然后，对bochsrc.txt中如下的必要配置项，进行设定：

| #内存megs: 32
#BIOS和VGABIOS的路径vgaromimage: file=/home/user/bochs/share/bochs/VGABIOS-lgpl-latest romimage: file=/home/user/bochs/share/bochs/BIOS-bochs-latest
#鼠标键盘mouse: enabled=0keyboard: keymap=/home/user/bochs/share/bochs/keymaps/x11-pc-us.map
#硬盘信息boot: diskata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14ata0-master: type=disk, path=&quot;/home/user/bochs/hd60M.img&quot;,mode=flat
log: bochs.txt |
| --- |

注意bochs配置文件最好是ASCII编码，UTF-8编码可能会出现a bochsrc option needs at least one parameter错误。

## 第3步，运行bochs

| bochs -f ~/bochs/bochsrc.txt回车（或选6） |
| --- |

成功运行的效果，如下图所示(左侧是bochs虚拟机，右侧是终端)：

![](RackMultipart20200521-4-xpdhab_html_1361e6b05b6beffc.png)

在终端中输入c，让bochs虚拟机继续运行，如下图所示(因为此时还没有写入引导程序，所以会引发panic)：



## 其他

将编译好的文件导入bochs虚拟硬盘，如下所示：

命令语法：

dd if=输入 of=输出 bs=设置缓冲区字节数 count=拷贝的块数 conv=？

样例：

dd if=mbr.bin of=~/bochs/hd60M.img bs=512 count=1 conv=notrunc

