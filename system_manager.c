//
// Created by Marco Lucas on 31/03/2023.
//
#include "processes.h"

//handler
void ctrlc_handler(int signal_num);

//setup
void create_shared_memory();

//threads
void *SENSOR_READER(void *id);
void *CONSOLE_READER(void *id);

//child processes
void WORKER(int id);
void ALERTS_WATCHER(int id);

int shmid;
char **sensors;

//shared memory region
SharedMemory *region;












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


//helper functions
int check_sensor();
int add_sensor();




void create_shared_memory()
{
    shmid = shmget(IPC_PRIVATE,sizeof(SharedMemory),IPC_CREAT|0777);
    region = shmat(shmid,NULL,0);
    region->header->first = (SensorData*) malloc(sizeof(SensorData));
    region->header->last = (SensorData*) malloc(sizeof(SensorData));

    //sh->sensors[0] = (char*)malloc(32*sizeof(char));
}





//system manager SIGINT
void ctrlc_handler(int signal_num)
{

    printf("Received SIGINT signal (Ctrl+C). Cleaning up resources and exiting...\n");

    // Cleanup resources here...

    //unlink named pipes
    unlink(SENSOR_PIPE);
    unlink(CONSOLE_PIPE);


    //terminate shared memory
    shmctl(shmid, IPC_RMID, NULL);
    // ...

    exit(0);
}



//DISPATCHER THREAD
void *DISPATCHER(void *id)
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


void *CONSOLE_READER(void *id)
{
    int fd;
    char *buffer;
    if ((fd = open(CONSOLE_PIPE, O_WRONLY)) < 0)
    {
        perror("Cannot open pipe for reading: ");
        pthread_exit(0);
    }
    MessageQueue *queue = (MessageQueue *)id;
    char msg[MAX_LEN_MSG];
    while(1)
    {
        read(fd, &buffer, sizeof(buffer));
        MessageQueueEnqueue(queue, msg);
    }

}




void *SENSOR_READER(void *id)
{
    int fd;
    char *buffer;
    if ((fd = open(SENSOR_PIPE, O_WRONLY)) < 0)
    {
        perror("Cannot open pipe for reading: ");
        pthread_exit(0);
    }
    MessageQueue *queue = (MessageQueue *)id;
    char msg[MAX_LEN_MSG];
    while(1)
    {
        read(fd, &buffer, sizeof(buffer));
        MessageQueueEnqueue(queue, msg);
    }
}

void WORKER(int id)
{

}

void ALERTS_WATCHER(int id)
{

}




int main(int argc, char *argv[])
{
    //setting up configurations
    setup();

    signal(SIGINT,ctrlc_handler);

    //creating named pipes
    if ((mkfifo(SENSOR_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) { perror("Cannot create SENSOR_PIPE: ");

    }
    if ((mkfifo(CONSOLE_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) { perror("Cannot create CONSOLE_PIPE: ");

    }


    //creating shared memory
    create_shared_memory();



    if(shmid == -1)
    {
        perror("Error in creating shared memory\n");
    }



    int WORKER_PIPE[2];
    int i;
    int id;


    //Initializing threads and child processes
    long thread_id[3];
    int process_id[N_WORKERS+1];

    pthread_t threads[3];
    pid_t worker;
    pid_t alerts_watcher;


    thread_id[0] = 0;
    pthread_create(&threads[0],NULL,DISPATCHER,&thread_id[0]);

    thread_id[1] = 1;
    pthread_create(&threads[1],NULL,SENSOR_READER,&thread_id[1]);

    thread_id[2] = 2;
    pthread_create(&threads[2],NULL,CONSOLE_READER,&thread_id[2]);

    for(int i = 0; i < 3; i++)
    {
        pthread_join(threads[i],NULL);
    }

    pipe(WORKER_PIPE);

    for(i = 0; i < N_WORKERS+1;i++)
    {
        if( (id = fork() ) == 0)
        {
            process_id[i] = i;
            if (i == 0)
                ALERTS_WATCHER(id);
            else
                WORKER(id);
        }
    }

}
