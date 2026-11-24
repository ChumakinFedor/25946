#define _POSIX_C_SOURCE 200112L

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
    int socket_fd;
    int status = EXIT_SUCCESS;

    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGPIPE, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_PATH);
    if (connect(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("connect " SOCKET_PATH);
        close(socket_fd);
        return EXIT_FAILURE;
    }

    for (;;) {
        ssize_t length = read(STDIN_FILENO, buffer, sizeof(buffer));

        if (length == 0) {
            break;
        }
        if (length == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            status = EXIT_FAILURE;
            break;
        }
        if (write_all(socket_fd, buffer, (size_t)length) == -1) {
            perror("write");
            status = EXIT_FAILURE;
            break;
        }
    }

    if (close(socket_fd) == -1) {
        perror("close");
        status = EXIT_FAILURE;
    }
    return status;
}
