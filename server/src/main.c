#include <pingServer.h>

#include <rawSocket.h>
#include <utils.h>

#include <stdbool.h>

int main(int ac, char** av) {
  if (ac != 2) {
    dprintf(2, "Error: bad args\nUsage: ./pingServer IP\n");
    exit(EXIT_FAILURE);
  }

  t_rawSocket* server = initializeRawSocket(av[1]);
  printf("Server start, listening on %s\n", server->_ipAddress);

  uint8_t buffer[200];
  while (true) {
    recvfrom(server->_sockfd, buffer, 200, MSG_WAITALL,
             (struct sockaddr*)&(server->_sockAddr), &((server->_socklen)));
    printf("\nDump packet:\n");
    hexDump(buffer, 200);
  }
}