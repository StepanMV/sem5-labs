#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

int main()
{
    pid_t pid;
    time_t parent_time;
    char message[256];

    if (mkfifo("myfifo", 0666) == -1)
    {
        perror("Failed to create FIFO");
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

        parent_time = time(NULL);
        snprintf(message, sizeof(message), "Parent time: %sPID: %d\n", ctime(&parent_time), getpid());

        sleep(5);

        int fd = open("myfifo", O_WRONLY);
        if (fd == -1)
        {
            perror("Failed to open FIFO for writing");
            exit(1);
        }

        write(fd, message, strlen(message) + 1);

        close(fd);
        unlink("myfifo");
    }
    else
    {

        int fd = open("myfifo", O_RDONLY);
        if (fd == -1)
        {
            perror("Failed to open FIFO for reading");
            exit(1);
        }

        read(fd, message, sizeof(message));

        time_t child_time = time(NULL);
        printf("Child time: %sReceived message: %s\n", ctime(&child_time), message);

        close(fd);
    }

    return 0;
}
