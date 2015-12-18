#include <stdlib.h>
#include "LIST.H"
#include "OBJECT.H"

/* LIST OPERATIONS */

LIST * createList() {

    LIST *list = (LIST *)malloc(sizeof(LIST));
    
    if (list != NULL)
    {
        list->head = NULL;
        list->rear = NULL;
        list->count = 0;
    }

    return list;
}

LIST_NODE * createNode(void *obj) {

    LIST_NODE * node = (LIST_NODE *)malloc(sizeof(LIST_NODE));
    
    if (node != NULL)
    {
        node->obj  = obj;
        node->next = NULL;
        node->prev = NULL;   
    }

    return node;
}


int add_toList(LIST *list,void *obj,LIST_NODE *node) {

	if (list == NULL) 
		return 1;
	   
    LIST_NODE *new_node = createNode(obj);
	
	if (new_node == NULL)
		return 1;

    if (list->count == 0) {

        list->head = list->rear = new_node;

    }else  {

        if (node == NULL)
            return 1;

		/* Insert at start */
        if (node == list->head) {

            new_node->next = node;
            node->prev = new_node;
            list->head = new_node;
		
		/* Insert at end */
        }else if (node == list->rear ) {
            
            new_node->prev = node;
            node->next = new_node;
            list->rear = new_node;

        /* Insert in middle */
        }else {
            
            new_node->next = node;
            new_node->prev = node->prev;
            node->prev = new_node;
            new_node->prev->next = new_node;
        
		}

    }

    (list->count)++;

    return 0;
}

int delete_fromList(LIST *list,LIST_NODE *node) {

    if (list == NULL || list->count == 0 || node == NULL)
        return 1;

 
    if (list->count == 1) {

        list->head=list->rear=NULL;    
    
    }else{


        /* Delete head node */
        if  (node == list->head) {
        
            list->head = node->next;
            (list->head)->prev = NULL;
        
        /* Delete rear node */
        }else if (node == list->rear) {
        
            list->rear = node->prev;
            (list->rear)->next = NULL;
        
        /* Delete other nodes */
        }else {
        
            (node->prev)->next = node->next;
            (node->next)->prev = node->prev;
        
        }
    }

    free(node);
    (list->count)--;
   
}

RESULT traverseList(LIST *list, int (*func)(void *,void *),void *data){


    LIST_NODE *tmp = list->head;
    RESULT res;
    int i=0;

    /* Initialize result*/
    res.result = 0;
    res.node = NULL;
    res.obj = NULL;

    if (tmp == NULL)
    {
        return res;
    }

    while (tmp) {

        i = func(tmp->obj,data);
        
        if (i == 1)
            break;
        
        tmp=tmp->next;
    }

    res.result = i;
    res.node = tmp;
    res.obj = tmp?tmp->obj:tmp;

    return res;
}

RESULT traverseList_rear(LIST *list, int (*func)(void *,void *),void *data){


    LIST_NODE *tmp = list->rear;
    RESULT res;
    int i=0;

    /* Initialize result*/
    res.result = 0;
    res.node = NULL;
    res.obj = NULL;

    if (tmp == NULL)
    {
        return res;
    }

    while (tmp) {

        i = func(tmp->obj,data);
        
        if (i == 1)
            break;
        
        tmp=tmp->prev;
    }

    res.result = i;
    res.node = tmp;
    res.obj = tmp?tmp->obj:tmp;

    return res;
}


RESULT searchList(LIST *list,void *search) {
    return traverseList(list,var_searchList,search);
}

/* Default Search */
int var_searchList(void *source,void *search) {
    if (search == source)
        return 1;
}

/* RESULT OPERATIONS */
RESULT getRear(LIST *list){
    RESULT res;
    

    res.result = 1;
    res.node = list->rear;
    res.obj = list->rear?(list->rear->obj):(list->rear);
    
    return res;
}

RESULT getHead(LIST *list){
    RESULT res;
    
    res.result = 1;
    res.node = list->head;
    res.obj = list->head?(list->head->obj):(list->head);

    return res;
}

void *getObject(RESULT *res){
    return res->obj;
}

LIST_NODE *getNode(RESULT *res){
    return res->node;
}

int getResult(RESULT *res) {
    return res->result;
}

void
move_node_result (LIST *list,RESULT *res) {
    if (res == NULL || list == NULL)
        return;

    LIST_NODE *node = res->node;
        
    if (node == NULL)
        return;

    if (list->rear == node)
        return;
    
    if (list->head == node) {
        list->head = node->next;
        list->head->prev = NULL;
    }else {
        (node->prev)->next = node->next;
        (node->next)->prev = node->prev;
    }

    node->next = NULL;
    node->prev = list->rear;
    list->rear->next = node;
    list->rear = node;

}
