#ifndef UTILS_H
#define UTILS_H

#include <ft_ping.h>
#include <netUtils.h>
#include <timeUtils.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct s_rawSocket t_rawSocket;

void exitProgram(const char* message, int code, bool hasErrno);
void printFirstLog(t_rawSocket* rawSocket, t_ping* ping);
void printLog(t_rawSocket* rawSocket, t_ping* ping, uint8_t ttl);

char* ft_strdup(const char* s);

#endif