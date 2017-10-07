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

void
close_sock(uint16_t sockfd, char *node)
{
    fprintf(stderr, "\nDisconnecting with %s...\n", node);
    close(sockfd);
}

int
safe_read(uint16_t sockfd, char *buffer)
{
    /* Handle errors while reading from a socket. */
    int bytes_read;

    bytes_read = read(sockfd, buffer, BUFSIZE);
    if (bytes_read < 0) {
        fprintf(stderr, "Error while reading from socket.\n");
        exit(EXIT_FAILURE);
    } else if (bytes_read == 0) {
#ifdef SERVER
        /* Connection terminated. */
        fprintf(stderr, "Seems like client has closed the connection.");
        close_sock(sockfd, "client");
#else
        fprintf(stderr, "Seems like server has closed the connection.");
        close_sock(sockfd, "server");
#endif
        return 0;
    }

    return bytes_read;
}

void
safe_write(char *buffer, int bytes_read, FILE *fp)
{
    /* Handle errors while writing to a file. */
    int bytes_written;

    /*
     * It is possible that fwrite may fail while writing.
     * Hence, we keep on attempting at writing until successful.
     */
    while(1) {
        bytes_written = fwrite(buffer, 1, bytes_read, fp);
        if (bytes_written == bytes_read)
            break;
    }
}
