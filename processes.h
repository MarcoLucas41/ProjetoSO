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
#include <sys/msg.h>
#include <fcntl.h>	// for O_ constants
#include <semaphore.h>
#include "string.h"
#include <pthread.h>
#include <time.h>
#include <stddef.h>
#include <ctype.h>


#ifndef PROJETOSO_PROCESSES_H
#define PROJETOSO_PROCESSES_H


#define MAX_LEN_MSG 100
#define SENSOR_PIPE  "sensor_pipe"
#define CONSOLE_PIPE "console_pipe"


void setup();
int QUEUE_SZ,N_WORKERS,MAX_KEYS,MAX_SENSORS,MAX_ALERTS;
FILE *system_config;
FILE *system_log;
char timestamp[10];
sem_t *mutex_log;

char* get_time()
{
    time_t seconds = time(NULL);;
    struct tm *timeStruct = localtime(&seconds);
    snprintf(timestamp,sizeof(timestamp), "%d:%d:%d", timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec);
    //printf("%s\n",timestamp);
    return timestamp;
}

void logging(char *message)
{
    system_log = fopen("/Users/marcolucas/sharedfoldervm/projetoSO/Log.txt", "a+");
    //system_log = fopen("/home/kamiguru/Desktop/so/proj/Log.txt", "a+");
    fprintf(system_log,"%s %s\n",get_time(),message);
    fclose(system_log);
}

void wrong_command(char * buffer)
{
    char message[MAX_LEN_MSG + 50];
    sem_wait(mutex_log);
    snprintf(message,MAX_LEN_MSG,"WRONG COMMAND => %s",buffer);
    logging(message);
    sem_post(mutex_log);
}

typedef struct {
    const int queue_sz;
    char *messages[MAX_LEN_MSG];
    int ri; //index do read
    int wi; //index do write
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} InternalQueue;


void InternalQueueInitial(InternalQueue *queue)
{
    memset(queue->messages, 0, sizeof(queue->messages));
    queue->ri = 0; //index do read
    queue->wi = 0; //index do write
    queue->count = 0;

    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
}

void InternalQueueEnqueue(InternalQueue *queue, const char *message)
{
    pthread_mutex_lock(&queue->lock);
    while (queue->count == QUEUE_SZ)
    {
        pthread_cond_wait(&queue->not_full, &queue->lock);
    }

    strncpy(queue->messages[queue->wi], message, MAX_LEN_MSG);
    queue->wi = (queue->wi + 1) % QUEUE_SZ;
    queue->count++;

    pthread_mutex_unlock(&queue->lock);
    pthread_cond_signal(&queue->not_empty);
}

void InternalQueueDequeue(InternalQueue *queue, char *message)
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
    int configurations[5];
    char line[10];
    system_config = fopen("/Users/marcolucas/sharedfoldervm/projetoSO/Config.txt", "r");
    //system_config = fopen("/home/kamiguru/Desktop/so/proj/Config.txt", "r");
    sem_unlink("MUTEX_LOG");
    mutex_log = sem_open("MUTEX_LOG",O_CREAT|O_EXCL,0700,1);

    sem_wait(mutex_log);
    system_log = fopen("/Users/marcolucas/sharedfoldervm/projetoSO/Log.txt", "w");
    //system_log = fopen("/home/kamiguru/Desktop/so/proj/Log.txt", "w");
    fprintf(system_log,"%s %s\n",get_time(),"HOME_IOT SIMULATOR STARTING");
    fclose(system_log);
    sem_post(mutex_log);

    int i = 0;
    while(fgets(line,10,system_config) != NULL)
    {
        //printf("%s",line);
        configurations[i] = atoi(line);
        i+=1;
    }
    if(configurations[0] < 1)
    {
        perror("QUEUE_SZ value should be >= 1!");
        fclose(system_config);
        exit(1);
    }
    else if(configurations[1] < 1)
    {
        perror("N_WORKERS value should be >= 1!");
        fclose(system_config);
        exit(1);
    }
    else if(configurations[2] < 1)
    {
        perror("MAX_KEYS value should be >= 1!");
        fclose(system_config);
        exit(1);
    }
    else if(configurations[3] < 1)
    {
        perror("MAX_SENSORS value should be >= 1!");
        fclose(system_config);
        exit(1);
    }
    else if(configurations[4] < 0)
    {
        perror("MAX_ALERTS value should be >= 0!");
        fclose(system_config);
        exit(1);
    }
    QUEUE_SZ = configurations[0];
    N_WORKERS = configurations[1];
    MAX_KEYS = configurations[2];
    MAX_SENSORS = configurations[3];
    MAX_ALERTS = configurations[4];
    fclose(system_config);
}




typedef struct SensorStats
{
    char *key;
    int latest_value;
    int min_value;
    int max_value;
    int total;
    double avg_values;
    int counter_updates_key;
    struct SensorStats *next;
}SensorStats;

typedef struct Alerts
{
    char *id;
    char *key;
    int min_value;
    int max_value;
    struct Alerts *next;
}Alerts;

typedef struct Sensors
{
    char *id;
    struct Sensors *next;
}Sensors;

typedef struct ArrayStats
{
    SensorStats *first;
    SensorStats *last;
}ArrayStats;

typedef struct ArrayAlerts
{
    Alerts *first;
    Alerts *last;
}ArrayAlerts;

typedef struct ArraySensors
{
    Sensors *first;
    Sensors *last;
}ArraySensors;


typedef struct SharedMemory
{


    //archive of keys and sensor data stats
    ArrayStats *header1;

    //archive of alerts
    ArrayAlerts *header2;

    //array of sensors that have communicated with system
    ArraySensors *header3;

    int alerts_counter,keys_counter,sensors_counter;

}SharedMemory;


void LIST_STATS(SensorStats *pointer)
{
    printf("KEY     LAST    MIN     MAX     AVG     COUNT\n");
    while(pointer != NULL)
    {
        printf("%s      %d      %d      %d      %.2lf      %d\n",pointer->key,pointer->latest_value,pointer->min_value,
               pointer->max_value,pointer->avg_values,pointer->counter_updates_key);
        pointer = pointer->next;
    }
}

void LIST_SENSORS(Sensors *pointer)
{
    printf("ID\n");
    while(pointer != NULL)
    {
        printf("%s\n",pointer->id);
        pointer = pointer->next;
    }
}

void LIST_ALERTS(Alerts *pointer)
{
    printf("ID      KEY     MIN     MAX\n");
    while(pointer != NULL)
    {
        printf("%s      %s      %d      %d\n",pointer->id,pointer->key,pointer->min_value,
               pointer->max_value);
        pointer = pointer->next;
    }
}

int CHECK_EMPTY_ARRAYSTATS(ArrayStats *pointer)
{
    if(pointer->first == NULL) return 1;
    else return 0;
}

int CHECK_EMPTY_ARRAYALERTS(ArrayAlerts *pointer)
{
    if(pointer->first == NULL) return 1;
    else return 0;
}

int CHECK_EMPTY_ARRAYSENSORS(ArraySensors *pointer)
{
    if(pointer->first == NULL) return 1;
    else return 0;
}

void RESET_ARRAYSTATS(ArrayStats *pointer)
{
    SensorStats *temp;
    while(!(CHECK_EMPTY_ARRAYSTATS(pointer)))
    {
        temp = pointer->first;
        pointer->first = pointer->first->next;
        free(temp);
    }
    pointer->last = NULL;
    //printf("OK\n");

}

void RESET_ARRAYALERTS(ArrayAlerts *pointer)
{
    Alerts *temp;
    while(!(CHECK_EMPTY_ARRAYALERTS(pointer)))
    {
        temp = pointer->first;
        pointer->first = pointer->first->next;
        free(temp);
    }
    pointer->last = NULL;
    //printf("OK\n");
}

void RESET_ARRAYSENSORS(ArraySensors *pointer)
{
    Sensors *temp;
    while(!(CHECK_EMPTY_ARRAYSENSORS(pointer)))
    {
        temp = pointer->first;
        pointer->first = pointer->first->next;
        free(temp);
    }
    pointer->last = NULL;
}

void REMOVE_ALERT(ArrayAlerts *pointer,char *id)
{
    Alerts *temp;
    Alerts *father;
    father = pointer->first;
    temp = father->next;
    while(temp != NULL || temp->id != id)
    {
        father = father->next;
        temp = temp->next;
    }
    father->next = temp->next;
    free(temp);

}
int UPDATE_STATS(ArrayStats *pointer, char*key, int value)
{
    SensorStats *temp;
    temp = pointer->first;
    if(temp == NULL)
    {
        return 0;
    }

    //SEARCH FOR KEY
    while(strcmp(temp->key,key) != 0)
    {
        temp = temp->next;
        if(temp == NULL)
        {
            return 0;
        }
    }
    if(temp != NULL)
    {
        temp->latest_value = value;
        if(value > temp->max_value)
        {
            temp->max_value = value;
        }
        if(value < temp->max_value)
        {
            temp->min_value = value;
        }
        temp->counter_updates_key += 1;
        temp->total += value;
        temp->avg_values = temp->total/temp->counter_updates_key;
        return 1;
    }
    else
    {
        return 0;
    }
}

void ADD_KEY(ArrayStats *pointer, char*key, int value)
{
    SensorStats *temp = (SensorStats *) malloc(sizeof(SensorStats));
    if(temp != NULL)
    {
        strcpy(temp->key,key);
        temp->latest_value = value;
        if(value > temp->max_value)
        {
            temp->max_value = value;
        }
        if(value < temp->max_value)
        {
            temp->min_value = value;
        }
        temp->counter_updates_key += 1;
        temp->total += value;
        temp->avg_values = temp->total/temp->counter_updates_key;
        temp->next = NULL;
        if(CHECK_EMPTY_ARRAYSTATS(pointer))
            pointer->first = temp;
        else pointer->last->next = temp;
        pointer->last = temp;
    }
}

void ADD_ALERT(ArrayAlerts *pointer,char *id, char*key, int min, int max)
{
    Alerts *temp;
    temp = (Alerts *) malloc(sizeof(Alerts));
    if (temp != NULL)
    {
        strcpy(temp->id,id);
        strcpy(temp->key,key);
        temp->next = NULL;
        if(CHECK_EMPTY_ARRAYALERTS(pointer))
            pointer->first = temp;
        else pointer->last->next = temp;
        pointer->last = temp;
    }
}
#endif //PROJETOSO_PROCESSES_H
