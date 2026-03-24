#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct s_list {
  void* data;
  struct s_list* next;
} t_list;

typedef struct s_ping t_ping;

void* galloc(size_t size, t_ping* ping);
void freeGarbage();
t_list* listNewElem(void* data, t_ping* ping);
void listPushFront(t_list** begin, t_list* elem, t_ping* ping);
size_t listLen(t_list* elem);

#endif