#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define myLog(...)                                                             \
  {                                                                            \
    printf(__VA_ARGS__);                                                       \
  }
#define pingLog(...)                                                           \
  {                                                                            \
    printf("\e[1;34m");                                                        \
    printf(__VA_ARGS__);                                                       \
    printf("\e[0m");                                                           \
  }

void exitError(const char* msg);
void hexDump(const uint8_t* buffer, const size_t size);

#endif