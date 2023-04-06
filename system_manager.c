//
// Created by Marco Lucas on 31/03/2023.
//
#include "processes.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


//setup
int* SETUP();

//threads
void *SENSOR_READER(void *id);
void *CONSOLE_READER(void *id);

//child processes
void WORKER(int id);
void ALERTS_WATCHER(int id);

#define QUEUE_SZ 10
#define MAX_LEN_MSG 100

typedef struct {
    char messages[QUEUE_SZ][MAX_LEN_MSG];
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

int* SETUP()
{
    int configurations[5];
    char line[10];
    FILE *f = fopen("~/Config.txt","r");
    int i = 0;
    while(fgets(line,10,f) != NULL)
    {
        configurations[i] = atoi(line);
        i+=1;
    }
    return configurations;
}

void *CONSOLE_READER(void *id)
{
    int fd;
    char *buffer;
    if ((fd = open(CONSOLE_PIPE, O_WRONLY)) < 0)
    {
        perror("Cannot open pipe for reading: ");
        pthread_exit(0);
    }
    while (1)
    {
        read(fd, &buffer, sizeof(buffer));
    }

}

void *SENSOR_READER(void *id)
{

}

void *SENSOR_THREAD(void *id)
{
    int sensor_id = *(int*)id;
    int sensor_fd;
    char buffer[20];

    if ((sensor_fd = open(SENSOR_PIPE, O_RDONLY)) < 0)
    {
        perror("Cannot open pipe for reading: ");
        pthread_exit(0);
    }

    while (1)
    {
        if (read(sensor_fd, buffer, sizeof(buffer)) > 0)
        {
            printf("Sensor %d received data: %s\n", sensor_id, buffer);
        }
    }
}

void WORKER(int id)
{

}

void ALERTS_WATCHER(int id)
{

}

//READER THREAD
void *READER_THREAD_FUNCTION(void *id)
{
    MessageQueue *queue = (MessageQueue *)id;
    char msg[MAX_LEN_MSG];

    while (1) {
        // Reading from sensor pipe or console input
        // ...

        // Enqueueing message
        MessageQueueEnqueue(queue, msg);
    }

    return NULL;
}

//DISPATCHER THREAD
void *DISPATCHER_THREAD_FUNCTION(void *id)
{
    MessageQueue *queue = (MessageQueue *)id;
    char msg[MAX_LEN_MSG];

    while (1) {
        // Dequeueing message
        MessageQueueDequeue(queue, msg);

        // Processing message
        // ...

        printf("The received message is: %s\n", msg);
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    if ((mkfifo(SENSOR_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) { perror("Cannot create SENSOR_PIPE: ");

    }
    if ((mkfifo(CONSOLE_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) { perror("Cannot create CONSOLE_PIPE: ");

    }

    //funcoes de destroir named pipes
    //unlink(SENSOR_PIPE);
    //unlink(SENSOR_PIPE);
    MessageQueue queue;
    MessageQueueInitial(&queue);

    //setting up configurations
    int *config = SETUP();
    int N_WORKERS = config[1];
    int MAX_SENSORS = config[3];

    int WORKER_PIPE[2];
    int i;
    int id;

    // Create the sensor threads
    int num_sensors = MAX_SENSORS;
    pthread_t sensor_threads[num_sensors];
    int sensor_ids[num_sensors];
    for (int i = 0; i < num_sensors; i++)
    {
        sensor_ids[i] = i;
        pthread_create(&sensor_threads[i], NULL, SENSOR_THREAD, &sensor_ids[i]);
    }


    //Initializing threads and child processes
    long thread_id[3];
    int child_id[N_WORKERS+MAX_SENSORS+1];

    pthread_t threads[3];
    pid_t workers[N_WORKERS];
    pid_t alerts_watcher;


    thread_id[0] = 0;
    pthread_create(&threads[0],NULL,DISPATCHER,&thread_id[0]);

    thread_id[1] = 1;
    pthread_create(&threads[0],NULL,SENSOR_READER,&thread_id[1]);

    thread_id[2] = 2;
    pthread_create(&threads[0],NULL,CONSOLE_READER,&thread_id[2]);

    pthread_t reader_thread, dispatcher_thread;
    pthread_create(&reader_thread, NULL, READER_THREAD_FUNCTION, &queue);
    pthread_create(&dispatcher_thread, NULL, DISPATCHER_THREAD_FUNCTION, &queue);

    pthread_join(reader_thread, NULL);
    pthread_join(dispatcher_thread, NULL);

    //signal(SIGINT,funcao de limpeza do sistema);

    //struct sigaction novo;
    //novo.sa_handler = ctrlc_handler;
    //sigfillset(&novo.sa_mask);
    //novo.sa_flags = 0;
    //sigaction(SIGINT,&novo,NULL);

    pipe(WORKER_PIPE);

    for(i = 3; i < N_WORKERS+1;i++)
    {
        if((id = fork() ) == 0)
        {
            if (i == N_WORKERS + 3)
                ALERTS_WATCHER(id);
            else

                WORKER(id);
        }
    }
}
