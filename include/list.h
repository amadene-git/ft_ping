#ifndef GABAGE_COLLECTOR_H
#define GABAGE_COLLECTOR_H

#include <stdlib.h>

typedef struct s_list {
  void* data;
  struct s_list* next;
} t_list;

void* galloc(size_t size);
void freeGarbage();
t_list* listNewElem(void* data);
void listPushFront(t_list** begin, t_list* elem);

#endif