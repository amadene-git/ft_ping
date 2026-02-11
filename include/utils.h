#ifndef UTILS_H
#define UTILS_H

#include <ft_ping.h>
#include <netUtils.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct s_rawSocket t_rawSocket;

void exitProgram(const char* message, int code, bool hasErrno);
void printFirstLog(t_rawSocket* rawSocket, t_ping* ping);
void printLog(t_rawSocket* rawSocket,
              t_ping* ping,
              uint8_t ttl,
              struct timeval rtt);
typedef struct s_list {
  void* data;
  struct s_list* next;
} t_list;

t_list* newElem(void* data);
void pushBack(t_list* begin, t_list* elem);

#endif