#include "csapp.h"
#include "LIST.H"
#include "OBJECT.H"

#define MAX_CACHE_SIZE (1024*1024*1024)

#ifndef CACHE_H
#define CACHE_H



typedef struct cache{
    LIST *ObjList;
    long totSize;
}CACHE;

sem_t cache_r;
sem_t cache_w;
sem_t lock;

int createCache(CACHE **cache);

int insert_intoCache(CACHE *cache,void *obj);

RESULT inCache(CACHE *cache,int (*func)(void *,void *),void *obj);

void update_cache_result (CACHE *,RESULT *) ;

#endif
