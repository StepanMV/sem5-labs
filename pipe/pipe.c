#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

int main()
{
    int pipe_fd[2];
    pid_t pid;
    time_t parent_time;
    char message[256];

    if (pipe(pipe_fd) == -1)
    {
        perror("Pipe creation failed");
        exit(1);
    }

    pid = fork();
    if (pid < 0)
    {
        perror("Fork failed");
        exit(1);
    }

    if (pid > 0)
    {

        close(pipe_fd[0]);

        parent_time = time(NULL);
        printf("Parent (PID: %d) time: %s", getpid(), ctime(&parent_time));
        snprintf(message, sizeof(message), "Parent (PID: %d) time: %s", getpid(), ctime(&parent_time));

        sleep(5);

        write(pipe_fd[1], message, strlen(message) + 1);

        close(pipe_fd[1]);
    }
    else
    {

        close(pipe_fd[1]);

        read(pipe_fd[0], message, sizeof(message));

        time_t child_time = time(NULL);
        printf("Child (PID: %d) time: %s", getpid(), ctime(&child_time));
        printf("RECV: %s\n", message);

        close(pipe_fd[0]);
    }

    return 0;
}
