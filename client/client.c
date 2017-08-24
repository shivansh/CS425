#include "standard.h"

uint16_t sockfd;

void
int_handler() {
  /* Handle SIGINT (Ctrl+C). */
  close(sockfd);
  exit(EXIT_FAILURE);
}

void
make_socket(char *server_ip, uint16_t port) {
  /* Subroutine to connect a socket with the machine
   * address and return its file descriptor.
   */
  struct sockaddr_in server;

  sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "Socket instantiation unsuccessful: ");
    exit(EXIT_FAILURE);
  }

  printf("Socket instantiated\n");
  server.sin_family      = AF_INET;
  server.sin_port        = htons(port);
  server.sin_addr.s_addr = inet_addr(server_ip);

  if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Error while connecting socket: ");
    exit(EXIT_FAILURE);
  }

  printf("Socket connected to server at port %d\n", port);
}

int
main(int argc, char **argv) {
  int  pid;
  int  server_port;
  int  wstatus;
  int  bytes_read;
  int  bytes_written;
  char server_ip[16];
  char password[256];
  char username[256];
  char filename[256];
  char buffer[BUFSIZE];
  FILE *fp;

  /* Disable buffering on standard output stream. */
  setbuf(stdout, NULL);

  /* Handle interrupts */
  signal(SIGINT, int_handler);

  if (argc != 3) {
    fprintf(stderr, "Usage: ./client username:password@<server_ip> <port>\n");
    exit(EXIT_FAILURE);
  }

  server_port = atoi(argv[2]);

  if ((pid = fork()) < 0) {
    fprintf(stderr, "Error on fork\n");
    close_sock(sockfd, "server");
    exit(EXIT_FAILURE);
  }

  else if (pid == 0) {
    /* This is the child process. It establishes a connection
     * with the server while parent process 'wait()'s on it.
     */

    /* Extract username, password and server IP from argv[1]. */
    int i = 0;
    bzero(username, sizeof(username));
    while (argv[1][i] != ':') {
      username[i] = argv[1][i];
      i++;
    }
    i++;

    int j = 0;
    bzero(password, sizeof(password));
    while (argv[1][i] != '@') {
      password[j] = argv[1][i];
      i++;
      j++;
    }
    i++;

    j = 0;
    bzero(server_ip, sizeof(server_ip));
    while(argv[1][i] != '\0') {
      server_ip[j] = argv[1][i];
      i++;
      j++;
    }

    make_socket(server_ip, server_port);
    bzero(buffer, BUFSIZE);

    /* Get the username and password from client. */
    sprintf(buffer, "%s", username);
    send(sockfd, buffer, strlen(username), 0);

    bzero(buffer, BUFSIZE);
    if (safe_read(sockfd, buffer))
      exit(EXIT_SUCCESS);

    if (!strcmp(buffer, "no_user")) {
      printf("Username \'%s\' does not exist."
             " Do you want to register? [y/n] ", username);
      char ans;
      scanf("%c", &ans);
      switch(ans) {
        case 'y':
        case 'Y':
          printf("Retype password: ");
          scanf("%s", password);
          bzero(buffer, BUFSIZE);
          sprintf(buffer, "%s %s\n", username, password);
          send(sockfd, buffer, strlen(buffer) + 1, 0);
          printf("Successfully registered!\n");
          close(sockfd);
          return 0;

        default:
          close(sockfd);
          return 0;
      }
    }

    sprintf(buffer, "%s", password);
    send(sockfd, buffer, strlen(password), 0);

    /* Server will send authentication status. */
    bzero(buffer, BUFSIZE);
    if (safe_read(sockfd, buffer))
      exit(EXIT_SUCCESS);

    if (!strncmp(buffer, "Hello", 5)) {
      printf("%s\n", buffer);
      bzero(buffer, BUFSIZE);

      printf("Enter the filename: ");
      scanf("%s", filename);
      sprintf(buffer, "%s", filename);
      send(sockfd, buffer, BUFSIZE, 0);

      bzero(buffer, BUFSIZE);
      if (safe_read(sockfd, buffer))
        exit(EXIT_SUCCESS);

      /* Receive file transfer initiation cue. */
      if (!strncmp(buffer, "Initiating", 10)) {
        fp = fopen(filename, "w");
        printf("+--------------------------+\n"
               "| Initiating file transfer |\n"
               "+--------------------------+\n");

        /* Receive file from the server in chunks. */

        while(1) {
          bzero(buffer, BUFSIZE);
          bytes_read = read(sockfd, buffer, BUFSIZE);
          /* printf("%s\n", buffer); */
          /* printf("%d\n", bytes_read); */

          if (bytes_read < 0) {
            fprintf(stderr, "Error while reading from socket\n");
            continue;
          }

          if (bytes_read < BUFSIZE) {
            safe_write(buffer, bytes_read, fp);
            break;
          }

          else {
            safe_write(buffer, bytes_read, fp);
          }
        }

        printf("+------------------------+\n"
               "| File transfer complete |\n"
               "+------------------------+\n");
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
    waitpid(pid, &wstatus, 0);
    if (wstatus != 0)
      fprintf(stderr, "Child process terminated with an error!\n");

    close_sock(sockfd, "server");
  }

  return 0;
}