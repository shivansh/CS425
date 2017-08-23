#include "standard.h"

#define MAX_CONN 5

uint16_t sockfd;
struct sockaddr_in client;
char buffer[BUFLEN];

void make_socket(uint16_t);
void process(uint16_t);

void int_handler() {
  /* Handle SIGINT (Ctrl+C). */
  close(sockfd);
  exit(EXIT_FAILURE);
}

void make_socket(uint16_t port) {
  /* Subroutine to bidn a socket with the machine
   * address and return its file descriptor.
   */

  sockfd = socket(PF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("Socket instantiation unsuccessful ");
    exit(EXIT_FAILURE);
  }

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
}

void process(uint16_t sockfd) {
  int bytes_read;
  int invalid = 0;
  char username[256];
  char password[256];
  char base_dir[256] = "./serve/";
  FILE *fp;

  bzero(buffer, BUFLEN);
  while(1) {
    if (safe_read(sockfd, buffer))
      break;

    /* Safely print data instead of dumping. */
    /* Authenticate the client's username. */
    sprintf(username, "%s", buffer);
    if (!strcmp(buffer, "shivansh")) {
      bzero(buffer, BUFLEN);
      if (safe_read(sockfd, buffer))
        break;
      sprintf(password, "%s", buffer);

      /* Authenticate the client's password. */
      if (!strcmp(password, "rai")) {
        sprintf(buffer, "Hello %s", username);
        send(sockfd, buffer, BUFLEN, 0);

        /* Client will now send the filename. */
        bzero(buffer, BUFLEN);
        if (safe_read(sockfd, buffer))
          break;

        /* Check if file exists. */
        strcat(base_dir, buffer);
        if (!access(base_dir, F_OK)) {
          /* Send file transfer initiation cue. */
          sprintf(buffer, "%s", "Initiating");
          send(sockfd, buffer, BUFLEN, 0);

          bzero(buffer, BUFLEN);
          fp = fopen(base_dir, "r");

          /* Start sending the file in chunks. */
          int counter = 0;
          int bytes_written;
          while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            bytes_written = send(sockfd, buffer, bytes_read, 0);
            /* Clear the buffer in case the size of next
             * read is less than the buffer size BUFLEN.
             */
            bzero(buffer, BUFLEN);
          }

          fclose(fp);
        }

        else {
          bzero(buffer, BUFLEN);
          sprintf(buffer, "%s", "+----------------+\n"
                                "| File not found |\n"
                                "+----------------+");
          send(sockfd, buffer, BUFLEN, 0);
        }
      }

    else invalid = 1;
    }

    if (invalid) {
      bzero(buffer, BUFLEN);
      sprintf(buffer, "%s", "+---------------------------+\n"
                            "| Authentication failure!!! |\n"
                            "+---------------------------+");
      send(sockfd, buffer, BUFLEN, 0);
    }

    bzero(buffer, BUFLEN);
  }
}

int main(int argc, char **argv) {
  uint16_t port;
  uint16_t client_sockfd;
  int client_len;
  int wstatus;
  pid_t pid;
  pid_t result;
  struct sockaddr_storage serverStorage;

  /* Disable buffering on standard output stream. */
  setbuf(stdout, NULL);

  /* Handle interrupts */
  signal(SIGINT, int_handler);

  if (argc != 2) {
    fprintf(stderr, "Usage: ./server <PORT>\nExiting!\n");
    exit(EXIT_FAILURE);
  }

  port   = atoi(argv[1]);
  make_socket(port);

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
      close(client_sockfd);
      exit(EXIT_FAILURE);
    }

    else if (pid == 0) {
      /* This is the child process. */
      process(client_sockfd);
      close(client_sockfd);
      exit(EXIT_SUCCESS);
    }

    else {
      /* This is the parent process.
       * We return immediately if no child has exited.
       */
      result = waitpid(pid, &wstatus, WNOHANG);
      close(client_sockfd);
    }

    /* Sleep for half a second. */
    usleep(500000);
  }

  close_sock(sockfd, "client");
  return 0;
}
