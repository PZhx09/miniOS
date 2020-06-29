#include "memory.h"
#include "bitmap.h"
#include "stdint.h"
#include "global.h"
#include "debug.h"
#include "print.h"
#include "string.h"

#define PG_SIZE 4096   //每页4KB

/***************  位图地址 ********************
 * 因为0xc009f000是内核主线程栈顶，0xc009e000是内核主线程的pcb.
 * 一个页框大小的位图可表示128M内存(1字节8个位表示8个页，4096*8*4KB=128MB), 
 * 位图位置安排在地址0xc009a000,本系统最大支持4个页框的位图,即512M 
 **********************************************/
#define MEM_BITMAP_BASE 0xc009a000
//跨过低端1MB的内存(那1MB是直接连续映射到了xc00fffff)
#define K_HEAP_START 0xc0100000
//PDE_IDX取得虚拟地址对应PDE的索引
//PTE_IDX取得去你地址对应PTE的索引
#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

//物理内存池，管理物理内存，分为用户内存池和内核内存池       与虚拟的区别是多了一个  内存池容量
struct pool {
    struct bitmap pool_bitmap;	 
    uint32_t phy_addr_start;	 
    uint32_t pool_size;		 // 内存池字节容量
};

struct pool kernel_pool, user_pool;      // 内核内存池，物理内存池
struct virtual_addr kernel_vaddr;	 // 此结构是用来给内核分配虚拟地址   目前就只有只有内核main一个线程

//初始化内存池，单位都是字节
static void mem_pool_init(uint32_t all_mem) {
    put_str("   mem_pool_init start\n");
    uint32_t page_table_size = PG_SIZE * 256;	  // 页表大小= 1页的页目录表+第0和第768个页目录项指向同一个页表+
                                                   // 第769~1022个页目录项共指向254个页表,第1023个页目录项指向页目录表不重复计算，共256个页框
    uint32_t used_mem = page_table_size + 0x100000;	  // 0x100000为低端1M内存
    uint32_t free_mem = all_mem - used_mem;
    uint16_t all_free_pages = free_mem / PG_SIZE;		  			  
    uint16_t kernel_free_pages = all_free_pages / 2;//对于真正的物理地址内存池，用户和内核这里均分好了
    uint16_t user_free_pages = all_free_pages - kernel_free_pages;



    uint32_t kbm_length = kernel_free_pages / 8;			  // 位图长度，这里是字节，一个位能表示一页，所以长度(字节)=空闲页数/8
    uint32_t ubm_length = user_free_pages / 8;			  

    uint32_t kp_start = used_mem;				  // 内核内存池起始地址
    uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;	  // 用户内存池起始地址

    kernel_pool.phy_addr_start = kp_start;
    user_pool.phy_addr_start   = up_start;

    kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
    user_pool.pool_size	 = user_free_pages * PG_SIZE;

    kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
    user_pool.pool_bitmap.btmp_bytes_len	  = ubm_length;

    //内核内存池起始地址
    kernel_pool.pool_bitmap.bits = (void*)MEM_BITMAP_BASE;
							       
    // 用户内存池的位图紧跟在内核内存池位图之后
    user_pool.pool_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length);
    /******************** 输出内存池信息 **********************/
    put_str("      kernel_pool_bitmap_start:");put_int((int)kernel_pool.pool_bitmap.bits);
    put_str(" kernel_pool_phy_addr_start:");put_int(kernel_pool.phy_addr_start);
    put_str("\n");
    put_str("      user_pool_bitmap_start:");put_int((int)user_pool.pool_bitmap.bits);
    put_str(" user_pool_phy_addr_start:");put_int(user_pool.phy_addr_start);
    put_str("\n");

    //初始化物理内存池的位图信息
    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    //初始化main线程虚拟地址内存池，也即内核虚拟地址池
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;      
    kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length + ubm_length);

    kernel_vaddr.vaddr_start = K_HEAP_START;//0xc0100000  1MB之外的地址
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    put_str("   mem_pool_init done\n");
}


//在pf表示的虚拟内存池中申请pg_cnt个虚拟页,成功则返回虚拟页的起始地址, 失败则返回NULL
static void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt) {
    int vaddr_start = 0, bit_idx_start = -1;
    uint32_t cnt = 0;
    if (pf == PF_KERNEL) {
        bit_idx_start  = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);//内核虚拟内存池中寻找连续pg_cnt个空页
        if (bit_idx_start == -1) {
            return NULL;
        }
        while(cnt < pg_cnt) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
        }
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
     } else { 
     // 用户虚拟内存池，待完善
     }
     return (void*)vaddr_start;
}

//得到虚拟地址vaddr对应的pde的虚拟地址 
uint32_t* pde_ptr(uint32_t vaddr) {
    //0xfffff000拆成两部分，前10位3ff表示第1023个页目录项，中间10个1表示指向页表的第1023项，实质上还是第1023项页目录项，即页目录表的首地址
    uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return pde;
}

// 得到虚拟地址vaddr对应的pte指针(指针都是指的是虚拟地址)
uint32_t* pte_ptr(uint32_t vaddr) {
    /* 先访问到页表自己 + 再用页目录项pde(页目录内页表的索引)做为pte的索引访问到页表 + 再用pte的索引做为页内偏移*/ 
    //页目录表起始地址+页目录项索引+页表项偏移地址
    //这里比较绕，看看操作系统真相还原里面实现的
    uint32_t* pte = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
    return pte;
}


//在m_pool指向的物理内存池中分配1个物理页,成功则返回页框的物理地址,失败则返回NULL
static void* palloc(struct pool* m_pool) {
    int bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1);    
    if (bit_idx == -1 ) {
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap, bit_idx, 1);    
    uint32_t page_phyaddr = (m_pool->phy_addr_start + (bit_idx * PG_SIZE));
    return (void*)page_phyaddr;
}

//页表中添加虚拟地址_vaddr与物理地址_page_phyaddr的映射
static void page_table_add(void* _vaddr, void* _page_phyaddr) {
    uint32_t vaddr = (uint32_t)_vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t* pde = pde_ptr(vaddr);
    uint32_t* pte = pte_ptr(vaddr);


    if (*pde & 0x00000001) {  // 页目录项和页表项的第0位为P,此处判断目录项是否存在

        if (!(*pte & 0x00000001)) {   // 只要是创建页表,pte就应该不存在,多判断一下以防万一
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);   //物理地址和虚拟地址映射成功
        } else {       
            //如果pte存在，说明已经页表项已经映射了 
            PANIC("pte repeat");
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);      
        }
    } else {             
        // 页目录项不存在,所以要先创建页目录再创建页表项.

        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);//内核内存池中分配一页作为页表

        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);//页目录项指向页表

        memset((void*)((int)pte & 0xfffff000), 0, PG_SIZE);//分配的物理页要做页表，所以先清零
         
        ASSERT(!(*pte & 0x00000001));
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);      // US=1,RW=1,P=1
    }
}

//分配pg_cnt个页空间,成功则返回起始虚拟地址,失败时返回NULL
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt) {
    ASSERT(pg_cnt > 0 && pg_cnt < 3840);
    //malloc_page 由上述实现的函数组合而成，主要分成三步:
    //1、vaddr_get申请虚拟内存
    //2、palloc申请物理内存
    //3、page_table_add将物理内存和虚拟内存映射
    void* vaddr_start = vaddr_get(pf, pg_cnt);
    if (vaddr_start == NULL) {
        return NULL;
    }

    uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;

    //因为虚拟地址是连续的,但物理地址可以是不连续的,所以逐个做映射
    while (cnt-- > 0) {
        void* page_phyaddr = palloc(mem_pool);
        if (page_phyaddr == NULL) {  
            //这里应该加一个内存回收，之后想想好了
            return NULL;
        }
        page_table_add((void*)vaddr, page_phyaddr); 
        vaddr += PG_SIZE;  

    }
    return vaddr_start;
}

/* 从内核物理内存池中申请pg_cnt页内存,成功则返回其虚拟地址,失败则返回NULL */
void* get_kernel_pages(uint32_t pg_cnt) {
    void* vaddr =  malloc_page(PF_KERNEL, pg_cnt);
    if (vaddr != NULL) {    //将分配的页先清零
        memset(vaddr, 0, pg_cnt * PG_SIZE);
    }
    return vaddr;
}

/* 内存管理部分初始化入口 */
void mem_init() {
    put_str("mem_init start\n");
    uint32_t mem_bytes_total = (*(uint32_t*)(0xb00));  //用物理地址直接取之前获得的内存容量
    mem_pool_init(mem_bytes_total);	  
    put_str("mem_init done\n");
}
