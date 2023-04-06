//
// Created by Marco Lucas on 30/03/2023.
//
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>	// for O_ constants
#include <semaphore.h>
#include "string.h"
#include <pthread.h>


#ifndef PROJETOSO_PROCESSES_H
#define PROJETOSO_PROCESSES_H


#define MAX_LEN_MSG 100
#define SENSOR_PIPE "sensor_pipe"
#define CONSOLE_PIPE "console_pipe"

void setup();
int config[5];
int QUEUE_SZ,N_WORKERS,MAX_KEYS,MAX_SENSORS,MAX_ALERTS;



typedef struct {
    const int queue_sz;
    char *messages[MAX_LEN_MSG];
    int ri; //index do read
    int wi; //index do write
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} MessageQueue;

void MessageQueueInitial(MessageQueue *queue)
{
    memset(queue->messages, 0, sizeof(queue->messages));
    queue->ri = 0; //index do read
    queue->wi = 0; //index do write
    queue->count = 0;

    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
}

void MessageQueueEnqueue(MessageQueue *queue, const char *message)
{
    pthread_mutex_lock(&queue->lock);
    while (queue->count == QUEUE_SZ) {
        pthread_cond_wait(&queue->not_full, &queue->lock);
    }

    strncpy(queue->messages[queue->wi], message, MAX_LEN_MSG);
    queue->wi = (queue->wi + 1) % QUEUE_SZ;
    queue->count++;

    pthread_mutex_unlock(&queue->lock);
    pthread_cond_signal(&queue->not_empty);
}

void MessageQueueDequeue(MessageQueue *queue, char *message)
{
    pthread_mutex_lock(&queue->lock);

    while (queue->count == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->lock);
    }

    strncpy(message, queue->messages[queue->ri], MAX_LEN_MSG);
    queue->messages[queue->ri][0] = '\0';

    // Clearing the message
    queue->ri = (queue->ri + 1) % QUEUE_SZ;
    queue->count--;

    pthread_mutex_unlock(&queue->lock);
    pthread_cond_signal(&queue->not_full);
}









void setup()
{
    char line[10];
    FILE *f = fopen("~/Config.txt","r");
    int i = 0;
    while(fgets(line,10,f) != NULL)
    {
        config[i] = atoi(line);
        if(config[0] < 1)
        {
            perror("QUEUE_SZ value should be >= 1!");
            exit(0);
        }
        else if(config[1] < 1)
        {
            perror("N_WORKERS value should be >= 1!");
            exit(0);
        }
        else if(config[2] < 1)
        {
            perror("MAX_KEYS value should be >= 1!");
            exit(0);
        }
        else if(config[3] < 1)
        {
            perror("MAX_SENSORS value should be >= 1!");
            exit(0);
        }
        else if(config[4] < 0)
        {
            perror("MAX_ALERTS value should be >= 0!");
            exit(0);
        }
        i+=1;
    }
    QUEUE_SZ = config[0];
    N_WORKERS = config[1];
    MAX_KEYS = config[2];
    MAX_SENSORS = config[3];
    MAX_ALERTS = config[4];
    fclose(f);
}




typedef struct SensorData
{
    char *key;
    int latest_value;
    int min_value;
    int max_value;
    double avg_values;
    int counter_updates_key;
    struct SensorData *next;

}SensorData;


typedef struct ArrayData
{
    SensorData *first;
    SensorData *last;
}ArrayData;

typedef struct SharedMemory
{
    ArrayData *header;
    char **sensors;

}SharedMemory;

#endif //PROJETOSO_PROCESSES_H
