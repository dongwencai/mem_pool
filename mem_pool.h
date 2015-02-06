#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__

#include <pthread.h>
#include <stdint.h>
#include "list_head.h"

typedef uint32_t mem_block_hdl_t;

typedef struct{
    pthread_rwlock_t rwlock;
    uint32_t bk_size;
    uint32_t bk_cnt;
}mem_pool_t;

typedef struct{
    struct list_head used_list;
    struct list_head free_list;
}mem_pool_list;

extern int mem_pool_create(mem_pool_t *pmem_pool[2],uint32_t block_size,uint32_t cnt);

extern mem_block_hdl_t mem_pool_get_block(mem_pool_t *pmem_pool[2]);

extern int mem_pool_free_block(mem_pool_t *pmem_pool[2],mem_block_hdl_t hbk);

extern int mem_pool_destory(mem_pool_t *pmem_pool[2]);

extern int print_mem_pool(mem_pool_t *pmem_pool[2]);

#endif
