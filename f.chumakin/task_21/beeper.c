#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    sigset_t signals;
    uintmax_t beep_count = 0;
    int received;
    int error;

    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGQUIT);

    /* Block both signals before waiting so neither can terminate the process. */
    if (sigprocmask(SIG_BLOCK, &signals, NULL) == -1) {
        perror("sigprocmask");
        return EXIT_FAILURE;
    }

    puts("Beeper started. SIGINT beeps; SIGQUIT prints the count and exits.");
    puts("Default terminal keys: Ctrl+C and Ctrl+\\.");
    if (fflush(stdout) == EOF) {
        perror("stdout");
        return EXIT_FAILURE;
    }

    for (;;) {
        /* Synchronous handling also lets us safely print the final count. */
        error = sigwait(&signals, &received);
        if (error != 0) {
            fprintf(stderr, "sigwait: %s\n", strerror(error));
            return EXIT_FAILURE;
        }

        if (received == SIGQUIT) {
            printf("\nBeeps: %" PRIuMAX "\n", beep_count);
            if (fflush(stdout) == EOF) {
                perror("stdout");
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }

        ssize_t written;
        do {
            written = write(STDOUT_FILENO, "\a", 1);
        } while (written == -1 && errno == EINTR);
        if (written != 1) {
            perror("write bell");
            return EXIT_FAILURE;
        }
        ++beep_count;
    }
}
