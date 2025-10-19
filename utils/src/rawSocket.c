#include <rawSocket.h>
#include <utils.h>

#include <arpa/inet.h>
#include <string.h>

t_rawSocket* initializeRawSocket(const char* bindingAddress) {
  t_rawSocket* server = NULL;
  if ((server = malloc(sizeof(t_rawSocket))) == NULL) {
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
  return server;
}
