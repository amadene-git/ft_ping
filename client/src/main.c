#include <ft_ping.h>

t_pingClient* initializeClient(const char* bindingAddress) {
  t_pingClient* server = NULL;
  if ((server = malloc(sizeof(t_pingClient))) == NULL) {
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

int main(int ac, char** av) {
  //   if (ac != 2) {
  //     dprintf(2, "Error: bad args\nUsage: ./pingServer IP\n");
  //     exit(EXIT_FAILURE);
  //   }

  (void)ac;
  (void)av;

  printf("Hello World !\n");
}