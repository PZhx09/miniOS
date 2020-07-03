[bits 32]
section .text
global switch_to
;调度上下文保护1)中断的上下文保护在intr entry和intr_exit那里  2)切换前后的上下文保护在这里
switch_to:
   ;栈中此处是返回地址
   ;ABI限制	       
   push esi
   push edi
   push ebx
   push ebp

   mov eax, [esp + 20]		 ;4个ABI寄存器，和返回地址，+20取得cur
   mov [eax], esp                ; 保存栈顶指针esp到task_struct的self_kstack字段

;------------------  以上是备份当前线程的环境，下面是恢复下一个线程的环境  ----------------
   mov eax, [esp + 24]		 ; +24拿到next
   mov esp, [eax]	;修改了栈

   pop ebp
   pop ebx
   pop edi
   pop esi
   ret				 ;修改了eip    中断进入的话，返回switch_to下面那句话继续执行。未由中断进入,第一次执行时会返回到kernel_thread
