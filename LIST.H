#include "csapp.h"
#ifndef LIST_H
#define LIST_H


sem_t list_r;
sem_t list_w;
int readers;

/* STRUCTURES */

typedef struct list_node{
    void *obj;
    struct list_node *prev;
    struct list_node *next; 
}LIST_NODE;

typedef struct List{
    LIST_NODE *head;
    LIST_NODE *rear;
    unsigned int count;
}LIST;

typedef struct Result{
    LIST_NODE *node;
    void *obj;
    int result;
}RESULT;


/* FUNCTIONS */

LIST * createList();

LIST_NODE * createNode(void *obj);

int add_toList(LIST *list,void *obj,LIST_NODE *node);

int delete_fromList(LIST *list,LIST_NODE *node);

void move_node_result (LIST *list,RESULT *res);

RESULT traverseList(LIST *list, int (*func)(void *,void *),void *data);

RESULT traverseList_rear(LIST *list, int (*func)(void *,void *),void *data);

int var_searchList(void *source,void *search);

RESULT searchList(LIST *list,void *search) ;

RESULT getRear(LIST *list);

RESULT getHead(LIST *list);

void *getObject(RESULT *res);

LIST_NODE *getNode(RESULT *res);

int getResult(RESULT *res);




#endif

