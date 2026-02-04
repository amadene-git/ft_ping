#include <rawSocket.h>
#include <utils.h>

#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
/*
int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);

void freeaddrinfo(struct addrinfo *res);

const char *gai_strerror(int errcode);
*/

t_rawSocket* initializeRawSocket(const char *host, t_rawSocket* server) {
  if ((server = malloc(sizeof(t_rawSocket))) == NULL) {
    exitError("malloc() failed");
  }

  if ((server->_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
    exitError("socket() failed");
  }

  resolveDNS(host, &server->_sockAddr);
  server->_sockAddr.sin_family = AF_INET;
  server->_socklen = sizeof(struct sockaddr_in);

//   if (bind(server->_sockfd, (struct sockaddr*)(&server->_sockAddr),
//            server->_socklen) == -1) {
//     exitError("bind() failed");
//   }
  return server;
}

int resolveDNS(const char *host, struct sockaddr_in *addrin) {
	struct addrinfo hints;
    struct addrinfo *result= NULL;
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_RAW; /* Datagram socket */
    hints.ai_protocol = IPPROTO_ICMP;          /* Any protocol */
   
	printf("try resolve DNS: %s\n", host);
	
	int ret = getaddrinfo(host, NULL, &hints, &result);

	if (ret != 0 || result == NULL){
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
	    exitError("get address failed");
	}

	*addrin = *(struct sockaddr_in *)result->ai_addr;
	char str_ip[100] = {0};
	inet_ntop(AF_INET, &addrin->sin_addr.s_addr,
                      &str_ip[0], 100);
	printf("resolve dns make: %s -> %s\n", host, str_ip);

	return 0;
}
