#include <stdlib.h>
#include <stdio.h>
#include "list.h"

List *list_create(){
  List *list = (List *)malloc(sizeof(List));
  if (list == NULL){
    fprintf(stderr, "ERROR list_create: malloc cannot allocate memory\n");
    return NULL;
  }
  list->head = NULL;
  list->size = 0;
  return list;
}

void list_destroy(List *list){
  if (list == NULL){
    return;
  }
  Node *current = list->head;
  while (current != NULL){
    Node *next = current->next;
    free(current);
    current = next;
  }
  free(list);
}

List * list_clone(List * l){
  List * clone = (List *)malloc(sizeof(*clone));
  if(l != NULL && l->head == NULL){
    clone->head = list_clone_aux(l->head, NULL);
    clone->size = l->size;
  }
  return clone;
}

Node * list_clone_aux(Node *l, Node*clone){
  if(l == NULL){
    return NULL;
  }else{
    clone = list_clone_aux(l->next, clone);
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL){
      fprintf(stderr, "ERROR list_add: malloc cannot allocate memory\n");
      exit(EXIT_FAILURE);
    }
    new_node->data = l;
    new_node->next = clone;
    return new_node;
  }
}

void list_add(List *list, void *data){
  Node *new_node = (Node *)malloc(sizeof(Node));
  if (new_node == NULL){
    fprintf(stderr, "ERROR list_add: malloc cannot allocate memory\n");
    return;
  }
  new_node->data = data;
  new_node->next = list->head;
  list->head = new_node;
  list->size++;
}

void list_delete(List *list, void *data, int (*compare)(void *, void *)){
  if (list == NULL || list->head == NULL){
    return;
  }

  Node *current = list->head;
  Node *prev = NULL;
  while (current != NULL){
    if (compare(current->data, data) == 0){
      if (prev == NULL){
        list->head = current->next;
      }else{
        prev->next = current->next;
      }
      free(current);
      list->size--;
      return;
    }
    prev = current;
    current = current->next;
  }
}

void list_print(List *l){
  if(l == NULL || l->head == NULL){
    printf("Empty list. Size: 0\n");
  }
  Node* current = l->head;
  while(current != NULL){
    printf("[%d]->", *(int *)current->data);
    current = current->next;
  }
  printf("NULL. Size: %d\n", l->size);
}

Node* mergesort(Node* l, int (*compare)(void *, void *)){
  if(l == NULL || l->next == NULL){
    return l;
  }else{
    Node *m = split(l);
    l = mergesort(l, compare);
    m = mergesort(m, compare);
    return merge(l, m, compare);
  }
}

Node* merge(Node *l, Node *m, int (*compare)(void *, void *)){
  if(l == NULL){
    return m;
  }else if(m == NULL){
    return l;
  }else{
    if(compare(l->data, m->data) < 0){
      l->next = merge(l->next, m, compare);
      return l;
    }else{
      m->next = merge(l, m->next, compare);
      return m;
    }
  }
}

Node* split(Node *l){
  if (l == NULL || l->next == NULL){
    return NULL;
  }

  Node* slow = l;
  Node* fast = l->next;

  while(fast != NULL){
    fast = fast->next;
    if (fast != NULL){
      slow = slow->next;
      fast = fast->next;
    }
  }

  Node* m = slow->next;
  slow->next = NULL;
  return m;
}