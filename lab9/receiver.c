#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define SHMEM_SIZE 64
#define SHMEM_FILE "shshshmemememe"

char *shared_memory = NULL;
struct sembuf lock = {0, -1, 0};
struct sembuf open = {0, 1, 0};

void handle_signal(int sig)
{
    if (shared_memory)
    {
        if (shmdt(shared_memory) < 0)
        {
            perror("shmdt");
        }
    }

    exit(0);
}

int main()
{

    key_t key = ftok(SHMEM_FILE, 1);
    if (key < 0)
    {
        perror("ftok");
        return -1;
    }

    int shmid = shmget(key, SHMEM_SIZE, 0666);
    if (shmid < 0)
    {
        perror("shmget");
        return -1;
    }

    int semid = semget(key, 1, 0666);
    if (semid == -1)
    {
        perror("semget() failed");
        return -1;
    }

    shared_memory = shmat(shmid, NULL, SHM_RDONLY);
    if (shared_memory == (void *)-1)
    {
        perror("shmat");
        return -1;
    }

    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    char local_copy[SHMEM_SIZE] = {0};

    while (1)
    {
        semop(semid, &lock, 1);
        strcpy(local_copy, shared_memory);
        semop(semid, &open, 1);

        time_t now = time(NULL);
        struct tm *current_time = localtime(&now);

        char time_buffer[32];
        strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", current_time);

        printf("[Time: %s | PID: %d] Shared Memory: %s\n", time_buffer, getpid(), local_copy);
    }

    return 0;
}
