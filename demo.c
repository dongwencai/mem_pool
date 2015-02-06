#include "mem_pool.h"
#include "config.h"
#include "stdio.h"
#include <unistd.h>

int main()
{
    int i;
    mem_pool_t *pmem_pool[2];
    mem_block_hdl_t hbk;
    mem_pool_create(pmem_pool,10,5);
    for(i = 0;i < 10;i ++)
    {
        hbk = mem_pool_get_block(pmem_pool);
    }
    print_mem_pool(pmem_pool);
    mem_pool_free_block(pmem_pool,2);
    print_mem_pool(pmem_pool);
    printf("+++++++++++++++++++\n");
    hbk = mem_pool_get_block(pmem_pool);
    print_mem_pool(pmem_pool);
    printf("hbk = :%d\n",hbk);
}
