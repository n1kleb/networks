#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT "0"

/*void timeout(int sockfd)
{
  struct timeval timeout;
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;

  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
              sizeof(timeout)) < 0)
      perror("setsockopt failed\n");

  if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
              sizeof(timeout)) < 0)
      perror("setsockopt failed\n");
}*/

int setup_socket() {
  int sock;
  struct addrinfo hints = {}, *info = NULL;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(NULL, PORT, &hints, &info);
  if ((sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) <
      0) {
    perror("Unable to create socket!");
    return 0;
  }
  if (bind(sock, info->ai_addr, (int)info->ai_addrlen)) {
    perror("Unable to bind socket!");
    return 0;
  }
  freeaddrinfo(info);

  struct sockaddr_in addr = {};
  unsigned int socklen = sizeof(addr);
  getsockname(sock, (struct sockaddr *)&addr, &socklen);
  unsigned short port = ntohs(addr.sin_port);
  printf("Opened socket on port %i...\n", port);

  return sock;
}

void handle_client(int socket) {

  int timeout = 5;
  struct timeval tv;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

  const int BUFFER_SIZE = 1024;

  struct sockaddr_in client = {};
  char buffer[BUFFER_SIZE];

  setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
  setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

  for (;;) {
    socklen_t length = sizeof(client);
    memset(buffer, 0, BUFFER_SIZE);

    int msglen = 0;
    if ((msglen = recvfrom(socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client,
                           &length)) < 0) {
      perror("Unable to recieve message!");
      break;
    }

    char response[512] = "";

    sprintf(response, "[%s:%d] (%d) -> %s\n", inet_ntoa(client.sin_addr),
            ntohs(client.sin_port), msglen, buffer);

    printf(response);
    sendto(socket, response, strlen(response), 0, (struct sockaddr *)&client, length);
  }
}

int main() {
  int socket = setup_socket();
  if (!socket) return 1;

  handle_client(socket);
  close(socket);
  return 0;
}
