#include <stdlib.h>
#include <stdio.h>
#include "list.h"

List *list_create()
{
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
