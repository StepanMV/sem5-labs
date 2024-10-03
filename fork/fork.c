#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void sigint_handler(int signum)
{
    printf("Received SIGINT (signal %d). Custom handler executed in process (PID: %d).\n", signum, getpid());
}

void sigterm_handler(int signum, siginfo_t *info, void *context)
{
    printf("Received SIGTERM (signal %d) from process %d.\n", signum, info->si_pid);
}

void on_exit_handler(void)
{
    printf("Program is exiting. Custom exit handler executed.\n");
}

int main()
{
    pid_t pid;

    if (atexit(on_exit_handler) != 0)
    {
        perror("atexit failed");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        perror("Error setting up SIGINT handler");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_sigaction = sigterm_handler;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGTERM, &sa, NULL) == -1)
    {
        perror("Error setting up SIGTERM handler");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        printf("This is the child process (PID: %d)\n", getpid());
        sleep(10);
        exit(0);
    }
    else
    {
        printf("This is the parent process (PID: %d). Created child with PID: %d\n", getpid(), pid);
        wait(NULL);
        printf("Child process has terminated.\n");
    }

    return 0;
}
