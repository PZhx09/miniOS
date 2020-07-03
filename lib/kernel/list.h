#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H
#include "global.h"

//offset详见https://blog.csdn.net/cxf100900/article/details/5625397?utm_source=blogxgwz3
//offset是求member在struct_type里的偏移
#define offset(struct_type,member) (int)(&((struct_type*)0)->member)
//链表元素地址-链表元素地址相对于首地址的偏移=数据结构首地址
#define elem2entry(struct_type, struct_member_name, elem_ptr) (struct_type*)((int)elem_ptr - offset(struct_type, struct_member_name))

//利用list_elem可以求得数据地址，所以这里不需要包含数据成员
//STL里面是链表里包着数据，这里实现是数据里包着链表
struct list_elem {
    struct list_elem* prev; 
    struct list_elem* next; 
};

//链表
struct list {
	//队首和队尾
    struct list_elem head;
    struct list_elem tail;
};

//自定义函数类型function,用于在list_traversal中做回调函数 
typedef bool (function)(struct list_elem*, int arg);

void list_init (struct list*);
void list_insert_before(struct list_elem* before, struct list_elem* elem);
void list_push(struct list* plist, struct list_elem* elem);
void list_iterate(struct list* plist);
void list_append(struct list* plist, struct list_elem* elem);  
void list_remove(struct list_elem* pelem);
struct list_elem* list_pop(struct list* plist);
bool list_empty(struct list* plist);
uint32_t list_len(struct list* plist);
struct list_elem* list_traversal(struct list* plist, function func, int arg);
bool elem_find(struct list* plist, struct list_elem* obj_elem);
#endif
