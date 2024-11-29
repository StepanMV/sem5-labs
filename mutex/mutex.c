#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_SIZE 20
#define NUM_READERS 10

char shared_array[ARRAY_SIZE];

int write_counter = 0;

pthread_mutex_t mutex;

void *writer_thread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);

        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            shared_array[i] = 'A' + (write_counter % 26);
        }
        write_counter++;
        printf("Writer updated array to: %s\n", shared_array);

        pthread_mutex_unlock(&mutex);

        sleep(3);
    }
    return NULL;
}

void *reader_thread(void *arg)
{
    long tid = *((long *)arg);

    while (1)
    {
        pthread_mutex_lock(&mutex);

        printf("Reader %ld reads array: %s\n", tid, shared_array);

        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
    return NULL;
}

int main()
{
    pthread_t writer;
    pthread_t readers[NUM_READERS];

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        perror("Mutex init failed");
        return 1;
    }

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        shared_array[i] = '-';
    }
    shared_array[ARRAY_SIZE - 1] = '\0';

    if (pthread_create(&writer, NULL, writer_thread, NULL) != 0)
    {
        perror("Failed to create writer thread");
        return 1;
    }

    for (long i = 0; i < NUM_READERS; i++)
    {
        if (pthread_create(&readers[i], NULL, reader_thread, (void *)&i) != 0)
        {
            perror("Failed to create reader thread");
            return 1;
        }
    }

    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++)
    {
        pthread_join(readers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    return 0;
}
