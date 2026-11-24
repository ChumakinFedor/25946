#define _POSIX_C_SOURCE 200112L

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int write_all(int fd, const unsigned char *buffer, size_t length)
{
    while (length > 0) {
        ssize_t written = write(fd, buffer, length);

        if (written == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (written == 0) {
            errno = EIO;
            return -1;
        }
        buffer += written;
        length -= (size_t)written;
    }
    return 0;
}

static int transfer(int input, int output, int uppercase)
{
    unsigned char buffer[4096];

    for (;;) {
        ssize_t length = read(input, buffer, sizeof(buffer));

        if (length == 0) {
            return 0;
        }
        if (length == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (uppercase) {
            for (ssize_t i = 0; i < length; ++i) {
                buffer[i] = (unsigned char)toupper(buffer[i]);
            }
        }
        if (write_all(output, buffer, (size_t)length) == -1) {
            return -1;
        }
    }
}

int main(void)
{
    int channel[2];
    int status = EXIT_SUCCESS;
    int child_status;
    pid_t child;
    pid_t waited;
    struct sigaction action = {0};

    /* Handle broken pipes as write errors so the parent can reap its child. */
    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGPIPE, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (pipe(channel) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        close(channel[0]);
        close(channel[1]);
        return EXIT_FAILURE;
    }
    if (child == 0) {
        close(channel[1]);
        if (transfer(channel[0], STDOUT_FILENO, 1) == -1) {
            perror("child transfer");
            status = EXIT_FAILURE;
        }
        close(channel[0]);
        _exit(status);
    }

    close(channel[0]);
    if (transfer(STDIN_FILENO, channel[1], 0) == -1) {
        perror("parent transfer");
        status = EXIT_FAILURE;
    }
    close(channel[1]);

    do {
        waited = waitpid(child, &child_status, 0);
    } while (waited == -1 && errno == EINTR);
    if (waited == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }
    if (!WIFEXITED(child_status) || WEXITSTATUS(child_status) != EXIT_SUCCESS) {
        status = EXIT_FAILURE;
    }
    return status;
}
