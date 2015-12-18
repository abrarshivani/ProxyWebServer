#include "CACHE.H"


int 
createCache(CACHE **cache)
{
    Sem_init(&cache_w,0,1);
    Sem_init(&cache_r,0,1);
    Sem_init(&lock,0,1);

    *cache = (CACHE *)malloc (sizeof(CACHE));

    if ((*cache) == NULL)
        return 1;

    (*cache)->ObjList = createList();
    (*cache)->totSize = 0L;

    if ((*cache)->ObjList == NULL)
    {
        free(cache);
        return 1;
    }

    return 0;
}

int 
insert_intoCache(CACHE *cache,void *obj) 
{

    long freeSize = 0;
    long size;
    long evictSize = 0;

    RESULT res;

    //Check if any parameter is null
    if (cache == NULL || obj==NULL)
        return 1;

    P(&cache_w);

    // readCount++;
    // if (readCount == 1)
    //  P(&cache_r);
    
    // V(&cache_w);

    size = ((OBJECT *)obj)->size;
    //Get amount of free space in  cache
    freeSize =  MAX_CACHE_SIZE - (cache->totSize + size);

    while (freeSize <= 0) {
        
        res = getHead(cache->ObjList);
        evictSize += deleteObj(getObject(&res));
        delete_fromList(cache->ObjList,getNode(&res));
        freeSize+=evictSize;
    
    }

    res = getHead(cache->ObjList);

    /* Calculate total size */
    cache->totSize+=size - evictSize;

   // printf("Cache count : %d",cache->ObjList->count);
    add_toList(cache->ObjList,obj,getNode(&res));

    
    V(&cache_w);



    return 0;
}

RESULT 
inCache(CACHE *cache,int (*func)(void *,void *),void *obj) {
    return traverseList_rear(cache->ObjList,func,obj);
}

void
update_cache_result (CACHE *cache,RESULT *res) {
    move_node_result(cache->ObjList,res);
}

