#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H
void panic_spin(char* filename, int line, char* func, char* condition);

//https://www.cnblogs.com/whiteyun/archive/2011/07/07/2099918.html   预处理器的预定义宏
//C语言中宏支持可变参数，..."表示定义的宏其参数可变。__VA_ARGS__ 是预处理器所支持的专用标识符，代表所有与省略号相对应的参数. 
//__DATE__：表示当前源文件编译时的日期，格式为：月/天/年（Mmm dd yyyy）。
//__FILE__：表示当前正在处理的源文件名称。
//__LINE__：表示当前正在处理的源文件的行，可以用#line指令修改。
//__STDC__：表示是ANSI C标准。只有在编译器选项指定了/Za，并且不是编译C++程序时，被定义为常整数1；否则未定义。
//__TIME__：表示当前源文件的最近编译时的时间，格式为：小时/分/秒（hh:mm:ss）。
//__TIMESTAMP__：表示当前源文件的最近修改日期和时间，格式为：Ddd Mmm dd hh:mm:ss yyyy，其中Ddd是星期的缩写。
#define PANIC(...) panic_spin (__FILE__, __LINE__, __func__, __VA_ARGS__)


#ifdef NDEBUG//这里宏有两个定义是为了后面不需要这个宏的时候，直接在gcc编译的时候加上-D NDEBUG参数就可以将宏给省去，减少宏占用的空间
   #define ASSERT(CONDITION) ((void)0)
#else
   #define ASSERT(CONDITION)                                      \
      if (CONDITION) {/* 条件为真，不进行任何操作 */} else {                                    \
  /* 符号#让编译器将宏的参数转化为字符串字面量 */		  \
	 PANIC(#CONDITION);                                       \
      }
#endif /*__NDEBUG */

#endif /*__KERNEL_DEBUG_H*/
