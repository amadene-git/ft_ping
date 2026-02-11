#include <list.h>
#include <utils.h>

static t_list** g_garbage = NULL;

void* galloc(size_t size) {
  void* ptr = NULL;
  ptr = malloc(size);
  if (ptr == NULL) {
    exitProgram("malloc() failed", errno, EXIT_FAILURE);
  }

  t_list* newElem = malloc(sizeof(t_list));
  if (newElem == NULL) {
    exitProgram("malloc() failed", errno, EXIT_FAILURE);
  }
  if (g_garbage == NULL) {
    g_garbage = malloc(sizeof(t_list*));
    if (g_garbage == NULL) {
      exitProgram("malloc() failed", errno, EXIT_FAILURE);
    }
    *g_garbage = NULL;
  }
  newElem->data = ptr;
  newElem->next = *g_garbage;
  *g_garbage = newElem;
  return ptr;
}

void freeGarbage() {
  t_list* next = NULL;
  while ((*g_garbage)->next) {
    next = (*g_garbage)->next;
    free((*g_garbage)->data);
    free((*g_garbage));
    (*g_garbage) = next;
  }
  free((*g_garbage)->data);
  free((*g_garbage));
  free(g_garbage);
}

t_list* listNewElem(void* data) {
  t_list* new = galloc(sizeof(t_list));
  new->data = data;
  new->next = NULL;
  return (new);
}

void listPushFront(t_list** begin, t_list* elem) {
  if (begin == NULL) {
    begin = galloc(sizeof(t_list*));
    *begin = NULL;
  }
  elem->next = *begin;
  *begin = elem;
}