#include <pingServer.h>

void exitError(const char* msg) {
  dprintf(2, "%s, ", msg); // TODO add variadic
  dprintf(2, " (errno: %s)\n", strerror(errno));

  // TODO clean garbage collector

  exit(EXIT_FAILURE);
}

t_pingServer* setupPingServer(const char* bindingAddress) {
  t_pingServer* server = NULL;
  if ((server = malloc(sizeof(t_pingServer))) == NULL) {
    exitError("malloc() failed");
  }

  if ((server->_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
    exitError("socket() failed");
  }
  server->_sockAddr.sin_family = AF_INET;
  server->_socklen = sizeof(struct sockaddr_in);

  int pton_ret = 0;
  if ((pton_ret = inet_pton(AF_INET, bindingAddress,
                            (void*)(&server->_sockAddr.sin_addr))) <= 0) {
    if (pton_ret == 0) {
      exitError("Error: Invalid Ip Address");
    } else {

      exitError("inet_pton() failed");
    }
  }
  bzero(server->_ipAddress, 16);
  strcpy(server->_ipAddress, bindingAddress);

  if (bind(server->_sockfd, (struct sockaddr*)(&server->_sockAddr),
           server->_socklen) == -1) {
    exitError("bind() failed");
  }
  printf("Server start, listening on %s\n", server->_ipAddress);
  return server;
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

int main(int ac, char** av) {
  if (ac != 2) {
    dprintf(2, "Error: bad args\nUsage: ./pingServer IP\n");
    exit(EXIT_FAILURE);
  }

  t_pingServer* server = setupPingServer(av[1]);

  uint8_t buffer[200];
  while (true) {
    recvfrom(server->_sockfd, buffer, 200, MSG_WAITALL,
             (struct sockaddr*)&(server->_sockAddr), &((server->_socklen)));
    printf("\nDump packet:\n");
    hexDump(buffer, 200);
  }
}