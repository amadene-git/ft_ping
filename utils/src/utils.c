#include <utils.h>

void exitError(const char* msg) {
  dprintf(2, "%s, ", msg); // TODO add variadic
  dprintf(2, " (errno: %s)\n", strerror(errno));

  // TODO clean garbage collector

  exit(EXIT_FAILURE);
}
