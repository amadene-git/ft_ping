#include <list.h>
#include <utils.h>

void* galloc(size_t size, t_ping* ping) {
  void* ptr = NULL;
  ptr = malloc(size);
  if (ptr == NULL) {
    exitProgram("malloc() failed", errno, true, ping);
  }

  t_list* newElem = malloc(sizeof(t_list));
  if (newElem == NULL) {
    exitProgram("malloc() failed", errno, true, ping);
  }
  if (ping->garbage == NULL) {
    ping->garbage = malloc(sizeof(t_list*));
    if (ping->garbage == NULL) {
      exitProgram("malloc() failed", errno, true, ping);
    }
    *ping->garbage = NULL;
  }
  newElem->data = ptr;
  newElem->next = *ping->garbage;
  *ping->garbage = newElem;
  return ptr;
}

void freeGarbage(t_ping* ping) {
  t_list* next = NULL;
  if (ping->garbage == NULL) {
    return;
  }
  while ((*ping->garbage)->next) {
    next = (*ping->garbage)->next;
    free((*ping->garbage)->data);
    free((*ping->garbage));
    (*ping->garbage) = next;
  }
  free((*ping->garbage)->data);
  free((*ping->garbage));
  free(ping->garbage);
}

t_list* listNewElem(void* data, t_ping* ping) {
  t_list* new = galloc(sizeof(t_list), ping);
  new->data = data;
  new->next = NULL;
  return (new);
}

void listPushFront(t_list** begin, t_list* elem, t_ping* ping) {
  if (begin == NULL) {
    begin = galloc(sizeof(t_list*), ping);
    *begin = NULL;
  }
  elem->next = *begin;
  *begin = elem;
}

size_t listLen(t_list* elem) {
  size_t len = 0;

  while (elem) {
    ++len;
    elem = elem->next;
  }
  return len;
}