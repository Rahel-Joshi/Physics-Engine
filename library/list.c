#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct list {
  void **data;
  size_t size;
  size_t capacity;
  free_func_t freer;
} list_t;

list_t *list_init(size_t initial_capacity, free_func_t freer) {
  list_t *list = malloc(sizeof(list_t));
  assert(list);

  list->data = malloc(sizeof(void *) * initial_capacity);
  assert(list->data);

  list->size = 0;
  list->capacity = initial_capacity;
  list->freer = freer;

  return list;
}

void list_free(list_t *list) {
  if (list->freer != NULL) {
    for (size_t i = 0; i < list_size(list); i++) {
      list->freer(list->data[i]);
    }
  }
  free(list->data);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index < list_size(list));
  return list->data[index];
}

void list_add(list_t *list, void *value) {
  assert(value != NULL);

  // case where the list is full
  if (list->size == list->capacity) {
    list_t *newList = list_init(list->capacity * 2, list->freer);
    for (size_t i = 0; i < list->capacity; i++) {
      newList->data[i] = list_get(list, i);
    }
    free(list->data);
    list->data = newList->data;
    list->capacity *= 2;
    free(newList);
  }

  list->data[list->size] = value;
  list->size++;
}

void *list_remove(list_t *list, size_t index) {
  assert(list_size(list) > 0);
  void *ans = list->data[index];

  // shift the rest of the content in list's data over
  for (size_t i = index; i < list_size(list) - 1; i++) {
    list->data[i] = list->data[i + 1];
  }
  list->size--;
  return ans;
}
