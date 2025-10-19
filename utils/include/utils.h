#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void exitError(const char* msg);
void hexDump(const uint8_t* buffer, const size_t size);

#endif