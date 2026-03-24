#ifndef UTILS_H
#define UTILS_H

#include <ft_ping.h>

// std
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

void exitProgram(const char* message, int code, bool hasErrno, t_ping* ping);

void printFirstLog(t_ping* ping);
void printLog(t_ping* ping, ssize_t nbBytesRecv, uint8_t ttl);
void printStats(t_ping* ping);

char* ft_strdup(const char* s, t_ping* ping);

#endif