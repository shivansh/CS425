#include "standard.h"

uint16_t make_socket(char*, uint16_t);

uint16_t make_socket(char *server_ip, uint16_t port) {
  /* Subroutine to connect a socket with the machine
   * address and return its file descriptor.
   */
  uint16_t sock;
  struct sockaddr_in server;

  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    fprintf(stderr, "Socket instantiation unsuccessful\n");
    exit(EXIT_FAILURE);
  }

  printf("Socket instantiated\n");
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = inet_addr(server_ip);

  if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Error while connecting socket ");
    exit(EXIT_FAILURE);
  }

  printf("Socket connected to server at port %d\n", port);
  return sock;
}

int main(int argc, char **argv) {
  int pid;
  int retval;
  int flag;
  int bytes_read;
  int server_port;
  uint16_t sockfd;
  char buffer[BUFLEN];
  char server_ip[16];
  char username[256];
  char password[256];
  char filename[256];
  FILE *fp;

  if (argc != 3) {
    fprintf(stderr, "Usage: ./client <server_ip> <port>\n");
    exit(EXIT_FAILURE);
  }

  sprintf(server_ip, "%s", argv[1]);
  server_port = atoi(argv[2]);

  if ((pid = fork()) < 0) {
    fprintf(stderr, "Error on fork\n");
    exit(EXIT_FAILURE);
  }

  else if (pid == 0) {
    /* This is the child process. It establishes a connection
     * with the server while parent process 'wait()'s on it.
     */
    sockfd = make_socket(server_ip, server_port);
    bzero(buffer, BUFLEN);

    /* Get the username and password from client. */
    printf("Username: ");
    scanf("%s", username);
    sprintf(buffer, "%s", username);
    send(sockfd, buffer, strlen(username), 0);
    printf("Password: ");
    scanf("%s", password);
    sprintf(buffer, "%s", password);
    send(sockfd, buffer, strlen(password), 0);

    /* Server will send authentication status. */
    bzero(buffer, BUFLEN);
    read(sockfd, buffer, BUFLEN);
    if (!strncmp(buffer, "Hello", 5)) {
      printf("%s\n", buffer);

      printf("Enter the filename: ");
      scanf("%s", filename);
      bzero(buffer, BUFLEN);
      sprintf(buffer, "%s", filename);
      send(sockfd, buffer, strlen(buffer), 0);

      bzero(buffer, strlen(buffer));
      read(sockfd, buffer, BUFLEN);

      /* Receive file transfer initiation cue. */
      if (!strcmp(buffer, "Initiating")) {
        fp = fopen(filename, "a");
        printf("+--------------------------+\n"
               "| Initiating file transfer |\n"
               "+--------------------------+\n");

        /* Receive file from server. */
        bzero(buffer, strlen(buffer));
        /* int bytes_read = read(sockfd, buffer, BUFLEN); */

        void *p;
        int bytes_written;
        while(1) {
          int bytes_read = read(sockfd, buffer, BUFLEN);
          if (bytes_read == 0)
            break;

          if (bytes_read < 0) {}

          p = buffer;
          while (bytes_read > 0) {
            bytes_written = fwrite(p, strlen(p), 1, fp);
            if (bytes_written < 0) {}
            bytes_read -= bytes_written;
            p += bytes_written;
          }
        }

        printf("+------------------------+\n"
               "| File transfer complete |\n"
               "+------------------------+\n");
        /* fwrite(buffer, 1, strlen(buffer), fp); */
        fclose(fp);
      }

      else
        printf("%s", buffer);

    }
    else
      printf("%s", buffer);
  }

  else {
    /* Parent process waits until the client
     * finishes communication with the server.
     */
    waitpid(pid, &retval, 0);
    if (retval != 0)
      fprintf(stderr, "Child process terminated with an error!\n");

    close_sock(sockfd, "server");
  }

  return 0;
}
