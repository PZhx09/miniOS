#include "string.h"
#include "global.h"
#include "debug.h"

// 将dst_起始的size个字节置为value  memset(目标首地址，值，大小)
void memset(void* dst_, uint8_t value, uint32_t size) {
   ASSERT(dst_ != NULL);
   uint8_t* dst = (uint8_t*)dst_;  //这里注意void*要转换为uint8_t  因为按字节来修改
   while (size-- > 0)
      *dst++ = value;
}

// 将src_起始的size个字节复制到dst_  memcpy(目标地址，源地址，大小)
void memcpy(void* dst_, const void* src_, uint32_t size) {
   ASSERT(dst_ != NULL && src_ != NULL);
   uint8_t* dst = dst_;
   const uint8_t* src = src_;
   while (size-- > 0)
      *dst++ = *src++;
}

// 连续比较以地址a_和地址b_开头的size个字节,若相等则返回0,若a_大于b_返回+1,否则返回-1   memcmp(目标字符串，源字符串，大小)
int memcmp(const void* a_, const void* b_, uint32_t size) {
   const char* a = a_;
   const char* b = b_;
   ASSERT(a != NULL || b != NULL);
   while (size-- > 0) {
      if(*a != *b) {
	 return *a > *b ? 1 : -1; 
      }
      a++;
      b++;
   }
   return 0;
}

// 将字符串从src_复制到dst_  strcpy(目标地址，源地址)
char* strcpy(char* dst_, const char* src_) {
   ASSERT(dst_ != NULL && src_ != NULL);
   char* r = dst_;		       // 用来返回目的字符串起始地址
   while((*dst_++ = *src_++));
   return r;
}

// 返回字符串长度 
uint32_t strlen(const char* str) {
   ASSERT(str != NULL);
   const char* p = str;
   while(*p++);
   return (p - str - 1);//这里要注意，别忘了减去首地址。不然返回的是字符串尾部的虚拟地址，没意义
}

// 比较两个字符串,若a_中的字符大于b_中的字符返回1,相等时返回0,否则返回-1. 
int8_t strcmp (const char* a, const char* b) {
   ASSERT(a != NULL && b != NULL);
   while (*a != 0 && *a == *b) {
      a++;
      b++;
   }
/* 如果*a小于*b就返回-1,否则就属于*a大于等于*b的情况。在后面的布尔表达式"*a > *b"中,
 * 若*a大于*b,表达式就等于1,否则就表达式不成立,也就是布尔值为0,恰恰表示*a等于*b */
   if(*a < *b)
      return -1;
   else if(*a > *b)
      return 1;
   else
      return 0;
}

// 从左到右查找字符串str中首次出现字符ch的地址   这里是返回的ch的虚拟地址 
char* strchr(const char* str, const uint8_t ch) {
   ASSERT(str != NULL);
   while (*str != 0) {
      if (*str == ch) {
	 return (char*)str;	    
      }
      str++;
   }
   return NULL;
}

//从后往前查找字符串str中首次出现字符ch的地址
char* strrchr(const char* str, const uint8_t ch) {
   ASSERT(str != NULL);
   const char* last_char = NULL;
   //从前向后遍历，一直保存着最后一个是ch的地址
   while (*str != 0) {
      if (*str == ch) {
	 last_char = str;
      }
      str++;
   }
   return (char*)last_char;
}

// 将字符串src_拼接到dst_后,返回拼接的串地址   strcat(目标字符串，源字符串)
char* strcat(char* dst_, const char* src_) {
   ASSERT(dst_ != NULL && src_ != NULL);
   char* dst = dst_;
   while (*dst++);
   --dst;     
   while((*dst++ = *src_++));	 
   return dst_;
}

// 在字符串str中查找指定字符ch出现的次数 
uint32_t strchrs(const char* str, uint8_t ch) {
   ASSERT(str != NULL);
   uint32_t ch_cnt = 0;
   const char* p = str;
   while(*p != 0) {
      if (*p == ch) {
	 ch_cnt++;
      }
      p++;
   }
   return ch_cnt;
}
