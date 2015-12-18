#include <string.h>
#include "OBJECT.H"
#include <stdlib.h>
#include "CACHE.H"

OBJECT * 
createObj(char *url,char *buf,long size)
{
    
    char *u,*b;
    OBJECT *obj;

    if (url == NULL || buf == NULL)
        return NULL;

    obj = (OBJECT *)malloc(sizeof(OBJECT));

    if (obj == NULL)
        return NULL;

    u = malloc(sizeof(char) * (strlen(url) + 1));
 
    if (u == NULL)
    {
        free(obj);
        return NULL;
    }

    b = malloc(sizeof(char) * size);

    if (b == NULL)
    {
        free(obj);
        free(u);
        return NULL;
    
    }

    /* Copy */
    strcpy(u,url);
    memcpy(b,buf,size);

    /* Initialize object */
    obj->url = u;
    obj->data = b;
    obj->size = size;

    return obj;
}

long 
deleteObj(OBJECT *obj) {
    
    if (obj == NULL)
        return 0;

    long size = obj->size;

    free(obj->url);
    free(obj->data);
    free(obj);
    
    return size;
}


int 
insert_Object(CACHE *cache,char *url,char *buf,long size) {
    
    OBJECT *obj;
    int returnVal = 1;

    obj = createObj(url,buf,size);
    
    if (obj == NULL)
        return returnVal;

    returnVal = insert_intoCache(cache,obj);

    return returnVal;

}

int 
matchURL(void *source,void *url) {

    char *s,*d;
    
    s = ((OBJECT *)source)->url;
    d =  ((OBJECT *)url)->url;

    return (!strcmp(s,d));
}

int 
searchCache(CACHE *cache,char *url,char **buf,int *size) {
    
	int ret_val=0;
    RESULT res;
    OBJECT obj;
    
    obj.url = url;

    P(&cache_r);
	readCount++;

	if (readCount == 1)
		P(&cache_w);
	V(&cache_r);
	

    P(&lock);
	res = inCache(cache,matchURL,&obj);
    V(&lock);

    if (getResult(&res)) {
        *size = ((OBJECT *)getObject(&res))->size;
		// memcpy(*buf , ((OBJECT *)getObject(&res))->data,*size);
        *buf = ((OBJECT *)getObject(&res))->data;
        
            P(&lock);
                update_cache_result(cache,&res);
            V(&lock);
    	
        ret_val=1;
    }

    

    P(&cache_r);
	readCount--;

	if (readCount == 0)
		V(&cache_w);
	
	V(&cache_r);

    return ret_val;
}

