#include <utils.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>

void exitError(const char* msg) {
  myLog("ft_ping: %s, ", msg); // TODO add variadic
  myLog(" (errno: %s)\n", strerror(errno));

  // TODO clean garbage collector

  exit(EXIT_FAILURE);
}

void hexDump(const uint8_t* buffer, const size_t size) {
  printf("0000   ");
  for (size_t i = 0; i < size; ++i) {
    printf("%02x ", buffer[i]);
    if ((i + 1) % 16 == 0) {
      printf(" [");
      for (size_t j = i - 15; j <= i; ++j) {
        if (isprint(buffer[j])) {
          printf("%c", buffer[j]);
        } else {
          printf(".");
        }
      }
      printf("]\n%04d   ", (int)i + 1);
    } else if ((i + 1) % 8 == 0) {
      printf(" ");
    }
  }
  printf("\n");
}
