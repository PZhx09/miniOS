#include "console.h"
#include "print.h"
#include "stdint.h"
#include "sync.h"
#include "thread.h"
static struct lock console_lock;    //屏幕资源的锁

//终端初始化
void console_init() {
  lock_init(&console_lock); 
}

//获取终端资源
void console_acquire() {
   lock_acquire(&console_lock);
}

//释放资源
void console_release() {
   lock_release(&console_lock);
}

//终端资源输出字符串
void console_put_str(char* str) {
   console_acquire(); 
   put_str(str); 
   console_release();
}

//终端资源输出字符
void console_put_char(uint8_t char_asci) {
   console_acquire(); 
   put_char(char_asci); 
   console_release();
}

//中孤单资源输出数字
void console_put_int(uint32_t num) {
   console_acquire(); 
   put_int(num); 
   console_release();
}

