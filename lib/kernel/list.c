#include "list.h"
#include "interrupt.h"

//初始化
void list_init (struct list* list) {
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

//在before前插入elem
void list_insert_before(struct list_elem* before, struct list_elem* elem) { 
    //这里必须关中断
    //CAS操作也行，但是不知道为什么x86这里没法用
    enum intr_status old_status = intr_disable();

    before->prev->next = elem; 
    elem->prev = before->prev;
    elem->next = before;
    before->prev = elem;

    intr_set_status(old_status);
}

//队首添加元素
void list_push(struct list* plist, struct list_elem* elem) {
    list_insert_before(plist->head.next, elem); 
}

//队尾添加元素
void list_append(struct list* plist, struct list_elem* elem) {
    list_insert_before(&plist->tail, elem);     // 在队尾的前面插入
}

//删除elem
void list_remove(struct list_elem* elem) {
    enum intr_status old_status = intr_disable();
   
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;

    intr_set_status(old_status);
}

//弹出队伍中的第一个元素
struct list_elem* list_pop(struct list* plist) {
    struct list_elem* elem = plist->head.next;
    list_remove(elem);
    return elem;
} 

//plist中查找ojb_elem成功返回true，否则返回false
bool elem_find(struct list* plist, struct list_elem* obj_elem) {
    struct list_elem* elem = plist->head.next;
    while (elem != &plist->tail) {
        if (elem == obj_elem) {
	         return true;
        }
        elem = elem->next;
    }
    return false;
}

//对plist中的所有元素，调用回调函数func，参数arg   查看是否有满足条件的elem
struct list_elem* list_traversal(struct list* plist, function func, int arg) {
    struct list_elem* elem = plist->head.next;
    if (list_empty(plist)) { 
        return NULL;
    }

    while (elem != &plist->tail) {
        if (func(elem, arg)) {		  //满足条件，返回
	         return elem;
        }					  
        elem = elem->next;	       
    }
    return NULL;
}

//双向链表长度
uint32_t list_len(struct list* plist) {
    struct list_elem* elem = plist->head.next;
    uint32_t length = 0;
    while (elem != &plist->tail) {
        length++; 
        elem = elem->next;
    }
    return length;
}

//判断双向链表是否为空
bool list_empty(struct list* plist) {		
    return (plist->head.next == &plist->tail ? true : false);
}
