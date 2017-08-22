#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFLEN 32

void int_handler(int sockfd) {
  /* Handle SIGINT (Ctrl+C). */
  close(sockfd);
  exit(EXIT_FAILURE);
}

void close_sock(uint16_t sockfd, char *node) {
  fprintf(stderr, "\nDisconnecting with %s...\n", node);
  close(sockfd);
}

void safe_read(uint16_t sockfd, char *buffer) {
  int bytes_read;

  bytes_read = read(sockfd, buffer, BUFLEN);

  if (bytes_read < 0) {
    fprintf(stderr, "Error while reading from socket\n");
    exit(EXIT_FAILURE);
  }

  else if (bytes_read == 0) {
    /* Client has terminated the connection. */
    close_sock(sockfd, "client");
    /* How to put break here ? */
  }
}
