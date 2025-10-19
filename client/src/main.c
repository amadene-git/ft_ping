#include <ft_ping.h>
#include <rawSocket.h>

#include <stdio.h>

int main(int ac, char** av) {
  if (ac != 2) {
    dprintf(2, "Error: bad args\nUsage: ./pingServer IP\n");
    return 1;
  }

  t_rawSocket* rawSocket = initializeRawSocket(av[1]);
  printf("ft_pîng: socket initialized\n");

  const char* buffer = "HELLO";
  int ret =
      sendto(rawSocket->_sockfd, buffer, 5, MSG_CONFIRM,
             (struct sockaddr*)(&rawSocket->_sockAddr), rawSocket->_socklen);

  printf("ft_pîng: send %d bytes\n", ret);

	
}