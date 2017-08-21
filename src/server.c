#include "standard.h"

#define MAX_CONN 5

uint16_t make_socket(uint16_t);
void process(uint16_t);

/* TODO Fix globals */
struct sockaddr_storage serverStorage;
struct sockaddr_in client;
char buffer[256];

uint16_t make_socket(uint16_t port) {
  /* Subroutine to bidn a socket with the machine
   * address and return its file descriptor.
   */
  uint16_t sockfd;

  sockfd = socket(PF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("Socket instantiation unsuccessful ");
    exit(EXIT_FAILURE);
  }

  /* TODO Add logging functionality */
  printf("Socket instantiated\n");
  /* The htons() function converts from
   * host byte order to network byte order.
   */
  client.sin_family      = AF_INET;
  client.sin_port        = htons(port);
  client.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (bind(sockfd, (struct sockaddr*)&client, sizeof(client)) < 0) {
    perror("Error while binding socket ");
    exit(EXIT_FAILURE);
  }

  printf("Socket bound to port %d\n", port);
  return sockfd;
}

void process(uint16_t sockfd) {
  int bytes_read;
  char username[256];
  char password[256];

  while(1) {
    bzero(buffer, BUFLEN);
    bytes_read = read(sockfd, buffer, BUFLEN);

    if (bytes_read < 0) {
      fprintf(stderr, "Error while reading from socket\n");
      exit(EXIT_FAILURE);
    }

    else if (bytes_read == 0) {
      /* Client has terminated the connection. */
      close_sock(sockfd, "client");
      break;
    }

    else {
      /* Safely print data instead of dumping. */
      /* Authenticate the client's username. */
      if (!strcmp(buffer, "shivansh")) {
        bzero(buffer, BUFLEN);
        read(sockfd, buffer, BUFLEN);
        if (!strcmp(buffer, "rai")) {
          printf("Client authenticated");
        }
      }
    }
  }
}

int main(int argc, char **argv) {
  uint16_t port;
  uint16_t sockfd;
  uint16_t client_sockfd;
  int client_len;
  int status;
  pid_t pid;
  pid_t result;

  /* Disable buffering on stdout stream. */
  setbuf(stdout, NULL);

  /* Handle interrupts */
  /* signal(SIGINT, int_handler(sockfd)); */
  /* signal(SIGCHLD, SIG_IGN); */

  if (argc != 2) {
    fprintf(stderr, "Usage: ./server <PORT>\nExiting!\n");
    exit(EXIT_FAILURE);
  }

  port   = atoi(argv[1]);
  sockfd = make_socket(port);

  if (listen(sockfd, MAX_CONN) == 0)
    printf("Listening on port %d\n", port);

  while(1) {
    client_len = sizeof(client);
    client_sockfd = accept(sockfd, (struct sockaddr*)&serverStorage,
        (socklen_t*)&client_len);

    if (client_sockfd < 0) {
      fprintf(stderr, "Socket accept unsuccessful ");
      exit(EXIT_FAILURE);
    }

    printf("+-------------------------------------+\n"
           "| Successfully established connection |\n"
           "+-------------------------------------+\n");

    if ((pid = fork()) < 0) {
      fprintf(stderr, "Error on fork\n");
      close_sock(client_sockfd, "client");
      exit(EXIT_FAILURE);
    }

    else if (pid == 0) {
      /* This is the child process. */

      sprintf(buffer, "+-------------------+\n"
                      "| Hello from server |\n"
                      "+-------------------+\n");
      write(client_sockfd, buffer, BUFLEN);
      process(client_sockfd);
      close(client_sockfd);
      exit(EXIT_SUCCESS);
    }

    else {
      /* This is the parent process.
       * We return immediately if no child has exited.
       */
      result = waitpid(pid, &status, WNOHANG);
      close(client_sockfd);
    }

    /* Sleep for half second. */
    sleep(1);
  }

  close(sockfd);
  return 0;
}
