#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int send_message(int sock, const char *message, struct addrinfo *info) {
  if (sendto(sock, message, strlen(message), 0, info->ai_addr,
             sizeof(*(info->ai_addr))) < 0) {
    perror("Unable to send message!");
    return 1;
  }

  const int BUFFER_SIZE = 1024;
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  socklen_t length = sizeof(*(info->ai_addr));

  int msglen = 0;
  if ((msglen = recvfrom(sock, buffer, BUFFER_SIZE, 0, info->ai_addr,
                         &length)) < 0) {
    perror("Unable to recieve message!");
    return 1;
  }

  printf("[SERVER]: %s", buffer);
  return 0;
}

int main(int argc, char const *argv[]) {
  if (argc < 4) {
    printf("Usage: [ip] [port] [message] <count> <delay>!\n");
    return 1;
  }

  const char *ip = argv[1];
  const char *port = argv[2];
  const char *message = argv[3];
  const int count = (argc >= 5) ? atoi(argv[4]) : 1;
  const int delay = (argc >= 6) ? atoi(argv[5]) : 1;

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("Unable to create socket!");
    return 1;
  }

  struct addrinfo hints, *info = NULL;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(ip, port, &hints, &info);
  if (!info) {
    perror("Unable to get destination info!");
    return 1;
  }

  for (int i = 0; i < count; i++) {
    printf("в„–%i: ", i + 1);
    send_message(sock, message, info);
    sleep(delay);
  }
  freeaddrinfo(info);
  return 0;
}
