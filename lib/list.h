#ifndef LIST_H_
#define LIST_H_

typedef struct _Node
{
  void *data;
  struct _Node *next;
} Node;

typedef struct List
{
  Node *head;
  int size;
} List;

List * list_create();
void list_destroy(List *list);
void list_add(List *list, void *data);
void list_delete(List *list, void *data, int (*compare)(void *, void *));

#endif
