#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFSIZE 1024

void close_sock(uint16_t sockfd, char *node) {
  fprintf(stderr, "\nDisconnecting with %s...\n", node);
  close(sockfd);
}

int safe_read(uint16_t sockfd, char *buffer) {
  int bytes_read;

  bytes_read = read(sockfd, buffer, BUFSIZE);
  if (bytes_read < 0) {
    fprintf(stderr, "Error while reading from socket\n");
    exit(EXIT_FAILURE);
  }

  else if (bytes_read == 0) {
    /* Connection terminated. */
    fprintf(stderr, "Connection terminated.");
    close_sock(sockfd, "client");
    return 1;
  }

  return 0;
}

void safe_write(char *buffer, int bytes_read, FILE *fp) {
  int bytes_written;

  while(1) {
    bytes_written = fwrite(buffer, 1, bytes_read, fp);
    if (bytes_written == bytes_read)
      break;
  }
}
