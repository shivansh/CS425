#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFLEN 128

void int_handler(int sockfd) {
  /* Handle SIGINT (Ctrl+C). */
  close(sockfd);
  exit(EXIT_FAILURE);
}

void close_sock(uint16_t sockfd, char *node) {
  fprintf(stderr, "\nDisconnecting with %s...\n", node);
  close(sockfd);
}
