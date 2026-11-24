#define _POSIX_C_SOURCE 200112L

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "./socket"

static int write_all(int fd, const unsigned char *buffer, size_t length)
{
    while (length > 0) {
        ssize_t written = write(fd, buffer, length);

        if (written == -1 && errno == EINTR) {
            continue;
        }
        if (written <= 0) {
            if (written == 0) {
                errno = EIO;
            }
            return -1;
        }
        buffer += written;
        length -= (size_t)written;
    }
    return 0;
}

int main(void)
{
    struct sockaddr_un address = {0};
    struct sigaction action = {0};
    unsigned char buffer[4096];
    int server_fd;
    int client_fd = -1;
    int status = EXIT_FAILURE;

    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGPIPE, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_PATH);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind " SOCKET_PATH);
        close(server_fd);
        return EXIT_FAILURE;
    }
    if (listen(server_fd, 1) == -1) {
        perror("listen");
        goto cleanup;
    }

    fprintf(stderr, "Waiting for a client on %s\n", SOCKET_PATH);
    do {
        client_fd = accept(server_fd, NULL, NULL);
    } while (client_fd == -1 && errno == EINTR);
    if (client_fd == -1) {
        perror("accept");
        goto cleanup;
    }

    for (;;) {
        ssize_t length = read(client_fd, buffer, sizeof(buffer));

        if (length == 0) {
            status = EXIT_SUCCESS;
            break;
        }
        if (length == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            break;
        }
        for (ssize_t i = 0; i < length; ++i) {
            buffer[i] = (unsigned char)toupper(buffer[i]);
        }
        if (write_all(STDOUT_FILENO, buffer, (size_t)length) == -1) {
            perror("write");
            break;
        }
    }

cleanup:
    if (client_fd != -1) {
        close(client_fd);
    }
    close(server_fd);
    if (unlink(SOCKET_PATH) == -1) {
        perror("unlink " SOCKET_PATH);
        status = EXIT_FAILURE;
    }
    return status;
}
