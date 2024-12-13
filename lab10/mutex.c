#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_SIZE 9
#define NUM_READERS 10

char shared_array[ARRAY_SIZE];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *writer_thread(void *arg)
{
    usleep(100000);
    for (int i = 0; i < ARRAY_SIZE - 1; i++)
    {
        pthread_mutex_lock(&mutex);
        shared_array[i] = 'a' + (i % 26);
        printf("Array updated\n");
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }

    return NULL;
}

void *reader_thread(void *arg)
{
    for (int i = 0; i < ARRAY_SIZE - 1; i++)
    {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        printf("Reader %ld reads array: %s\n", pthread_self(), shared_array);
        pthread_mutex_unlock(&mutex);
        // usleep(1010000);
    }

    return NULL;
}

int main()
{
    pthread_t writer;
    pthread_t readers[NUM_READERS];

    shared_array[ARRAY_SIZE - 1] = '\0';

    if (pthread_create(&writer, NULL, writer_thread, NULL) != 0)
    {
        perror("Failed to create writer thread");
        return 1;
    }

    for (int i = 0; i < NUM_READERS; i++)
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
