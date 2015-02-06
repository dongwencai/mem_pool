#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mem_pool.h"
#include "gui_types.h"
#include "common.h"
#include <string.h>
#include <pthread.h>


static mem_pool_t *mem_pool_create_ex(uint32_t block_size,uint32_t cnt);
static mem_block_hdl_t mem_pool_get_block_ex(mem_pool_t *pmem_pool);
static int mem_pool_free_block_ex(mem_pool_t *pmem_pool,mem_block_hdl_t hbk);
static int mem_pool_destory_ex(mem_pool_t *pmem_pool);

int mem_pool_create(mem_pool_t *pmem_pool[2],uint32_t block_size,uint32_t cnt)
{
    mem_pool_list *pcontext;
    uint32_t i;
    pmem_pool[0] = mem_pool_create_ex(block_size,cnt); 
    pmem_pool[1] = mem_pool_create_ex(sizeof(mem_pool_list),cnt); 
    if(!pmem_pool[0] || !pmem_pool[1])  goto err;
    pcontext =(mem_pool_list *)(pmem_pool[1] + sizeof(mem_pool_t));

    pcontext->used_list.prev = pcontext->used_list.next = &pcontext->used_list;
    pcontext->free_list.prev = pcontext->free_list.next = &pcontext->free_list;
    for(i = 0 ;i < cnt - 1 ;i ++)
    {
        list_add(&pcontext[i + 1].free_list,&pcontext[i].free_list); 
    }
    return SUC;
err:
    if(pmem_pool[0])
    {
        pthread_rwlock_destroy(&pmem_pool[0]->rwlock);
        free(pmem_pool[0]);
    }
    if(pmem_pool[1])
    {
        pthread_rwlock_destroy(&pmem_pool[1]->rwlock);
        free(pmem_pool[1]);
    }
    return FAIL;
}

static mem_pool_t *mem_pool_create_ex(uint32_t block_size,uint32_t cnt)
{
    int fd;
    void *ptr;
    uint32_t pool_len,mem_pool_len;
    mem_pool_t *pmem_pool;
    pmem_pool = (mem_pool_t *)malloc(block_size * cnt + sizeof(mem_pool_t));
    if(pmem_pool)
    {
        pthread_rwlock_init(&pmem_pool->rwlock,NULL);
        pmem_pool->bk_size = block_size;
        pmem_pool->bk_cnt = cnt;
        return pmem_pool;
    }
err:
    return NULL;
}

mem_block_hdl_t mem_pool_get_block(mem_pool_t *pmem_pool[2])
{
    if(pmem_pool[0] && pmem_pool[1])
    {
        return mem_pool_get_block_ex(pmem_pool[1]);
    }
    return 0;
}
static mem_block_hdl_t mem_pool_get_block_ex(mem_pool_t *pmem_pool)
{
    mem_block_hdl_t hmem = 0;
    mem_pool_list *pcontext;
    pcontext = (mem_pool_list *)(pmem_pool + sizeof(mem_pool_t));
    pthread_rwlock_wrlock(&pmem_pool->rwlock); 
    if(!list_empty(&pcontext[0].free_list))
    {
        hmem = ((uint32_t)pcontext[0].free_list.next - (uint32_t)pcontext) / sizeof(mem_pool_list);
        if(hmem)
        {
            list_add(&pcontext[hmem].used_list,&pcontext[0].used_list);
            list_del(&pcontext[hmem].free_list);
        }
    }
    pthread_rwlock_unlock(&pmem_pool->rwlock); 
    return hmem;
}

int mem_pool_free_block(mem_pool_t *pmem_pool[2],mem_block_hdl_t hbk)
{
    if(pmem_pool[0] && pmem_pool[1] && hbk)
    {
        return mem_pool_free_block_ex(pmem_pool[1],hbk);
    }
    return FAIL;
}

static int mem_pool_free_block_ex(mem_pool_t *pmem_pool,mem_block_hdl_t hbk)
{
    mem_pool_list *pcontext;
    pcontext = (mem_pool_list *)(pmem_pool + sizeof(mem_pool_t));
    pthread_rwlock_wrlock(&pmem_pool->rwlock); 
    list_add(&pcontext[hbk].free_list,&pcontext[0].free_list);
    list_del(&pcontext[hbk].used_list);
    pthread_rwlock_unlock(&pmem_pool->rwlock); 
    return SUC;
}

int mem_pool_destory(mem_pool_t *pmem_pool[2])
{
    mem_pool_destory_ex(pmem_pool[0]);
    mem_pool_destory_ex(pmem_pool[1]);
    return SUC;
}

static int mem_pool_destory_ex(mem_pool_t *pmem_pool)
{
    int ret;
    ret = pthread_rwlock_destroy(&pmem_pool->rwlock);
    free(pmem_pool);
    return ret;
}

#ifdef DEBUG
int print_mem_pool(mem_pool_t *pmem_pool[2])
{
    mem_pool_list *pcontext,*pinfo;
    pcontext =(mem_pool_list *) (pmem_pool[1] + sizeof(mem_pool_t));
    list_for_each_entry(pinfo,&pcontext[0].used_list,used_list)
    {
        printf("%x\n",pinfo);
    }
    printf("free_list\n");
    list_for_each_entry(pinfo,&pcontext[0].free_list,free_list)
    {
        printf("%x\n",pinfo);
    }
}
#else
int print_mem_pool(mem_pool_t *pmem_pool[2])
{
}
#endif
