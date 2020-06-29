#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"

//位图初始化
void bitmap_init(struct bitmap* btmp){
    
    memset(btmp->bits,0,btmp->btmp_bytes_len);
}

//判断bit_idx位，是1返回true，是0返回false
bool bitmap_scan_test(struct bitmap* btmp,uint32_t bit_idx){

    uint32_t byte_idx=bit_idx/8;//字节索引
    uint32_t bit_idx_in_byte=bit_idx % 8;//在该字节中位的索引
    return (btmp->bits[byte_idx]&(BITMAP_MASK<<bit_idx_in_byte));
}

//申请连续cnt个位，成功返回下标，否则返回-1  注意是连续cnt个，返回的是32位无符号整型bit_idx
int bitmap_scan(struct bitmap* btmp,uint32_t cnt){
	uint32_t idx_byte = 0;//用来记录空闲位所在字节

	while((btmp->bits[idx_byte]==0xff)&&(idx_byte < btmp->btmp_bytes_len))
		idx_byte++;

    if (idx_byte == btmp->btmp_bytes_len) {  // 位图中无可用空间		
          return -1;
    }


    int idx_bit = 0;
    //不是0xff说明有空闲位，找到字节中的空闲位
    while ((uint8_t)(BITMAP_MASK << idx_bit) & btmp->bits[idx_byte]) { 
	    idx_bit++;
    }
	 
    int bit_idx_start = idx_byte * 8 + idx_bit;    
    if (cnt == 1) {
        return bit_idx_start;
    }

    uint32_t bit_left = (btmp->btmp_bytes_len * 8 - bit_idx_start);   //位图中剩余位
    uint32_t next_bit = bit_idx_start + 1;
    uint32_t count = 1;	      

    bit_idx_start = -1;	      // 先将其置为-1,若找不到连续的位就直接返回
    while (bit_left-- > 0) {
        if (!(bitmap_scan_test(btmp, next_bit))) {	 // 若next_bit为0
	        count++;
        } else {
	        count = 0;
        }
        if (count == cnt) {	    
	        bit_idx_start = next_bit - cnt + 1;
	        break;
        }
        next_bit++;          
    }
    return bit_idx_start;
}

//设置位图中bit_idx位为value   value目前也就是只有0和1
void bitmap_set(struct bitmap* btmp,uint32_t bit_idx,int8_t value){
	ASSERT((value == 0) || (value == 1));
    uint32_t byte_idx = bit_idx / 8;    
    uint32_t bit_idx_in_byte  = bit_idx % 8;    


    if (value) {		      // 如果value为1
        btmp->bits[byte_idx] |= (BITMAP_MASK << bit_idx_in_byte);
    } else {		      // 若为0
        btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_idx_in_byte);
    }

}
