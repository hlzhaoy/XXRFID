#include "list.h"
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

list InitList()
{
	list l = (list)malloc(sizeof(* l));
	if (l == NULL) {
		return NULL;
	}

	memset(l, 0, sizeof(* l));
	return l;
}

list InsertHead(list l, list t) 
{
	t->next = l->next;
	l->next = t;

	return l;
}

/*
* brief:
*/
list DeleteList(list l, list t)
{
	for (list tmp = l; tmp != NULL; tmp = tmp->next) {
		if (tmp->next == t) {
			tmp->next = t->next;
			free(t->data);
			free(t);
			break;
		}
	}

	return l;
}

/*
* brief: 
*/

list DeleteListByData(list l, void* data)
{
	for (list head = l; head->next != NULL; head = head->next) {
		if (head->next->data == data) {
			list tmp = head->next;
			head->next = tmp->next;
			free(tmp->data);
			free(tmp);
			break;
		}
	}

	return l;
}

/*
* brief:
*/

bool IsEmpty(list l)
{
	if (l->next == NULL) {
		return true;
	}

	return false;
}

/*
* brief:
*/
int GetListCount(list head)
{
	int count = 0;
	for (list t = head; t->next != NULL; t = t->next) {
		count ++;
	}

	return count;
}

/*
* brief:
*/

void* GetListTailData(list head)
{
	if (IsEmpty(head) == true) {
		return NULL;
	}

	for (list t = head; ; t = t->next) {
		if (t->next->next == NULL) {
			void* data = t->next->data;
			free(t->next);
			t->next = NULL;
			
			return data;
		} 
	}

	return NULL;
}

#ifdef __cplusplus
}
#endif