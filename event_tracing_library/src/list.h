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
Node* mergesort(Node* l, int (*compare)(void *, void *));
Node* merge(Node *l, Node *m, int (*compare)(void *, void *));
Node* split(Node *l);
List* list_clone(List *l);
Node* list_clone_aux(Node *l, Node *clone);

#endif
