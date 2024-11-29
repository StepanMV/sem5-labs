#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#define SHMEM_SIZE 64
#define SHMEM_FILE "shshshmemememe"

char *addr = NULL;
int shmid = -1;

void handle_signal(int sig)
{
    printf("Stopping the process writing to the shared memory (signal %d)\n", sig);

    if (addr != NULL)
    {
        if (shmdt(addr) < 0)
        {
            perror("Error detaching shared memory");
            exit(1);
        }
    }

    if (shmid != -1)
    {
        if (shmctl(shmid, IPC_RMID, NULL) < 0)
        {
            perror("Error removing shared memory segment");
            exit(1);
        }
    }

    if (remove(SHMEM_FILE) == -1)
    {
        perror("Error removing shared memory file");
        exit(1);
    }

    exit(0);
}

int main(int argc, char **argv)
{
    (void)argc, (void)argv;

    int fd = open(SHMEM_FILE, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("Create file error");
        return 1;
    }
    close(fd);

    key_t key = ftok(SHMEM_FILE, 1);
    if (key < 0)
    {
        perror("Ftok error");
        return -1;
    }

    shmid = shmget(key, SHMEM_SIZE, 0666 | IPC_CREAT);
    if (shmid < 0)
    {
        perror("Shmid error");
        return -1;
    }

    addr = shmat(shmid, NULL, 0);
    if (addr == (void *)-1)
    {
        perror("Shmat error");
        return -1;
    }

    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    while (1)
    {
        sleep(1);

        char timeStr[SHMEM_SIZE] = {0};
        char result[2 * SHMEM_SIZE] = {0};

        time_t ts = time(NULL);
        struct tm curTime = *localtime(&ts);

        strftime(timeStr, sizeof(timeStr), "%H:%M:%S;", &curTime);
        snprintf(result, sizeof(result), "Process: %d; %s", getpid(), timeStr);

        strcpy(addr, result);
    }

    return 0;
}
