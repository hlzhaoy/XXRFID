#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct node* list;

struct node {
	void* data;
	list next;
};

list InitList();

list InsertHead(list l, list t) ;

list DeleteList(list l, list t);

list DeleteListByData(list l, void* data);

bool IsEmpty(list l);

int GetListCount(list head);

void* GetListTailData(list head);

#ifdef __cplusplus
}
#endif

#endif /* LIST_H */