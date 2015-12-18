#ifndef OBJECT_H
#define OBJECT_H

typedef struct object{
    char *url;
    char *data;
    long  size;
}OBJECT;

int readCount;
OBJECT * createObj(char *,char *,long);
long deleteObj(OBJECT *);


#endif

