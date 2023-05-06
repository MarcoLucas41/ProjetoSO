//
// Marco Lucas 2021219146
// Bruno Almeida 2021237081

// Sistemas Operativos
// Licenciatura em Engenharia Informática 2022/2023
//
#include "processes.h"


//SIGINT handler
void ctrlc_handler(int signal_num);

void create_shared_memory();

//threads
void *SENSOR_READER(void *id);
void *CONSOLE_READER(void *id);

//child processes
void WORKER(int id,int i,int*pipe);
void ALERTS_WATCHER(int id);

int shmid, msqid;
pid_t parent;


//array of unnamed pipes and worker status(so that dispatcher knows which worker is free)
int **worker_pipes;
int *worker_status;
pid_t *process_id;

//semaphores
sem_t *mutex_shm_sem;
sem_t *thread_mutex;

//Initializing threads
long thread_id[3];
pthread_t threads[3];
int *destroy;

//shared memory region
SharedMemory *region;

//internal queue
InternalQueue queue;


void create_shared_memory()
{
    shmid = shmget(IPC_PRIVATE,sizeof(SharedMemory),IPC_CREAT|0777);
    if(shmid == -1)
    {
        perror("Error in creating shared memory\n");
        exit(1);
    }
    region = shmat(shmid,NULL,0);
    if(region == (void *) -1)
    {
        perror("Error in allocating shared memory\n");
        exit(1);
    }

    region->header1 = (ArrayStats *) malloc(sizeof(ArrayStats));
    region->header2 = (ArrayAlerts *) malloc(sizeof(ArrayAlerts));
    region->header3 = (ArraySensors *) malloc(sizeof(ArraySensors));

    region->header1->first = NULL;
    region->header1->last = NULL;

    region->header2->first = NULL;
    region->header2->last = NULL;

    region->header3->first = NULL;
    region->header3->last = NULL;

}




//system manager SIGINT
void ctrlc_handler(int signal_num)
{

    //THERE ARE 80 BYTES DEFINETLY LOST HERE
    // Cleanup resources here...
    if(parent == getpid())
    {
        printf("Inside!\n");
        sem_wait(mutex_log);
        logging("SIGNAL SIGINT RECEIVED");
        sem_post(mutex_log);

        //ending threads
        printf("Destroy set to %d\n",*destroy);
        *destroy = 1;
        printf("Destroy set to %d\n",*destroy);

        for(int i = 0; i < 3; i++)
        {
            pthread_join(threads[i],NULL);
        }
        free(destroy);
        if(sem_close(thread_mutex) != 0){fprintf(stderr, "sem_close() failed. errno:%d\n",errno);}
        if(sem_unlink("THREAD_MUTEX") != 0){fprintf(stderr, "sem_unlink() failed. errno:%d\n",errno);}

        //unlink named pipes
        unlink(SENSOR_PIPE);
        unlink(CONSOLE_PIPE);
        /*
        //APRESENTAR CONTEUDO DA INTERNAL QUEUE

        pthread_cond_destroy(&queue.not_empty);
        pthread_cond_destroy(&queue.not_full);
        pthread_mutex_destroy(&queue.lock);


        //closing unnamed pipes
        for(int i = 0; i < N_WORKERS; i++)
        {
            close(worker_pipes[i][0]);
            close(worker_pipes[i][1]);
            free(worker_pipes[i]);
        }
        free(worker_pipes);
        free(worker_status);

        //waiting for child processes to finish
        while(wait(NULL) > 0);
        sem_wait(mutex_log);
        logging("WORKERS AND ALERTS WATCHER TERMINATED");
        sem_post(mutex_log);
        free(process_id);

        //closing shm semaphore
        if(sem_close(mutex_shm_sem) != 0){fprintf(stderr, "sem_close() failed. errno:%d\n",errno);}
        if(sem_unlink("MUTEX_SHM_SEM") != 0){fprintf(stderr, "sem_unlink() failed. errno:%d\n",errno);}

        //cleaning sensor ids,stats and alerts arrays

        RESET_ARRAYSTATS(region->header1);
        free(region->header1->first);
        free(region->header1->last);
        free(region->header1);
        RESET_ARRAYALERTS(region->header2);
        free(region->header2->first);
        free(region->header2->last);
        free(region->header2);
        RESET_ARRAYSENSORS(region->header3);
        free(region->header3->first);
        free(region->header3->last);
        free(region->header3);


        //terminate shared memory and message queue
        //shmdt(region);
        shmctl(shmid, IPC_RMID, NULL);
        msgctl(msqid, IPC_RMID, NULL);
        */
        //closing log semaphore
        sem_wait(mutex_log);
        logging("HOME_IOT SIMULATOR CLOSING");
        sem_post(mutex_log);

        if(sem_close(mutex_log) != 0){fprintf(stderr, "sem_close() failed. errno:%d\n",errno);}
        if(sem_unlink("MUTEX_LOG") != 0){fprintf(stderr, "sem_unlink() failed. errno:%d\n",errno);}

        //exiting program
        exit(0);
    }
}



//DISPATCHER THREAD
void *DISPATCHER(void *id)
{
    printf("Starting DISPATCHER\n");

    id = (int*)id;
    //InternalQueue *iq = (InternalQueue *)id;
    char msg[MAX_LEN_MSG];

    while(*destroy == 0)
    {
        // Dequeueing message
        //InternalQueueDequeue(iq, msg);

        // Processing message
        /*
        for(int i = 0; i < N_WORKERS; i++)
        {
            if(worker_status[i] == 0)
            {
                write(worker_pipes[i][1],&msg, sizeof(msg));
                break;
            }
        }
        */
    }

    sem_wait(thread_mutex);
    printf("Ending DISPATCHER\n");
    sem_post(thread_mutex);
    pthread_exit(NULL);
}

void *SENSOR_READER(void *id)
{
    printf("Starting SENSOR_READER\n");
    int fd,nread;
    printf("OPENING SENSOR PIPE FOR READING\n");
    if ( ( fd = open(SENSOR_PIPE, O_NONBLOCK) ) < 0)
    {
        perror("Cannot open sensor pipe for reading: ");
        pthread_exit(NULL);
    }
    char buffer[MAX_LEN_MSG];
    printf("after OPENING SENSOR PIPE FOR READING\n");
    //InternalQueue *iq = (InternalQueue *)id;

    while(*destroy == 0)
    {
        nread = read(fd, &buffer, sizeof(buffer));
        if(nread > 0)
        {
            printf("outra coisa - %d - %s\n",nread,buffer);
        }
        //InternalQueueEnqueue(iq, buffer);
    }
    sem_wait(thread_mutex);
    printf("Ending SENSOR_READER\n");
    sem_post(thread_mutex);
    pthread_exit(NULL);
}

void *CONSOLE_READER(void *id)
{
    printf("Starting CONSOLE_READER\n");

    int fd,nread;
    printf("OPENING CONSOLE PIPE FOR READING\n");
    if ( ( fd = open(CONSOLE_PIPE, O_NONBLOCK) ) < 0)
    {
        perror("Cannot open console pipe for reading: ");
        pthread_exit(NULL);
    }
    char buffer[MAX_LEN_MSG];

    printf("after OPENING CONSOLE PIPE FOR READING\n");
    //InternalQueue *iq = (InternalQueue *)id;

    while(*destroy == 0)
    {
        nread = read(fd, &buffer, sizeof(buffer));
        if(nread > 0)
        {
            printf("alguma coisa - %d - %s\n",nread,buffer);
        }

        //InternalQueueEnqueue(iq, buffer);
    }
    sem_wait(thread_mutex);
    printf("Ending CONSOLE_READER\n");
    sem_post(thread_mutex);
    pthread_exit(NULL);

}


void WORKER(int id,int i,int*pipe)
{

    char buffer[MAX_LEN_MSG];
    char copy[MAX_LEN_MSG];
    char copy2[MAX_LEN_MSG];
    char temp[MAX_LEN_MSG];
    char *ptr;
    int wrong = 0;
    close(pipe[1]);
    while(1)
    {
        read(pipe[0], &buffer, sizeof(buffer));
        worker_status[i] = 1;   //worker is busy
        for(int j = 0; j < MAX_LEN_MSG; j++)
        {
            if(isalnum(buffer[j]) == 0)
            {
                wrong = 1;
                break;
            }
            else if(buffer[j] == '\0')
            {
                break;
            }
        }

        if(wrong == 1)
        {
            wrong_command(buffer);
            worker_status[i] = 0;
            continue;

        }
        strcpy(copy,buffer);
        strcpy(copy2,copy);
        strcpy(temp,copy2);

        char *delim = "\0";
        ptr = strtok(buffer, delim);

        //dealing with user console commands
        if(strcmp(ptr,"stats") == 0)
        {
            sem_wait(mutex_shm_sem);
            LIST_STATS(region->header1->first);
            sem_post(mutex_shm_sem);
            worker_status[i] = 0;
            continue;
        }
        else if(strcmp(ptr,"reset") == 0)
        {
            sem_wait(mutex_shm_sem);
            RESET_ARRAYSTATS(region->header1);
            RESET_ARRAYSENSORS(region->header3);
            region->sensors_counter = 0;
            region->keys_counter = 0;
            sem_post(mutex_shm_sem);
            printf("OK\n");
            worker_status[i] = 0;
            continue;
        }
        else if( strcmp(ptr,"list_alerts") == 0)
        {
            sem_wait(mutex_shm_sem);
            LIST_ALERTS(region->header2->first);
            sem_post(mutex_shm_sem);
            worker_status[i] = 0;
            continue;
        }
        else if(strcmp(ptr,"sensors") == 0)
        {
            sem_wait(mutex_shm_sem);
            LIST_SENSORS(region->header3->first);
            sem_post(mutex_shm_sem);
            worker_status[i] = 0;
            continue;
        }
        else
        {
            delim = " ";
            ptr = strtok(copy, delim);
            if(strcmp(ptr,"add_alert") == 0 )
            {
                char id[32],key[32];
                int min,max;
                ptr = strtok(NULL, delim);
                if(ptr == NULL)
                {
                    wrong_command(temp);
                    worker_status[i] = 0;
                    continue;
                }
                strcpy(id,ptr);
                ptr = strtok(NULL, delim);
                if(ptr == NULL)
                {
                    wrong_command(temp);
                    worker_status[i] = 0;
                    continue;
                }
                strcpy(key,ptr);
                ptr = strtok(NULL, delim);
                if(ptr == NULL)
                {
                    wrong_command(temp);
                    worker_status[i] = 0;
                    continue;
                }
                min = atoi(ptr);
                ptr = strtok(NULL, "\0");
                max = atoi(ptr);
                if(min == 0 || max == 0)
                {
                    wrong_command(temp);
                    worker_status[i] = 0;
                    continue;
                }
                sem_wait(mutex_shm_sem);
                if(region->alerts_counter == MAX_ALERTS)
                {
                    sem_wait(mutex_log);
                    logging("!LIMIT OF ALERTS REACHED!");
                    sem_post(mutex_log);
                    sem_post(mutex_shm_sem);
                    worker_status[i] = 0;
                    continue;
                }
                ADD_ALERT(region->header2,id,key,min,max);
                region->alerts_counter += 1;
                sem_post(mutex_shm_sem);
                worker_status[i] = 0;
                continue;
            }
            else if(strcmp(ptr,"remove_alert") == 0)
            {
                char id[32];
                ptr = strtok(NULL, delim);
                if(ptr == NULL)
                {
                    wrong_command(temp);
                    worker_status[i] = 0;
                    continue;
                }
                strcpy(id,ptr);
                sem_wait(mutex_shm_sem);
                REMOVE_ALERT(region->header2,id);
                region->alerts_counter -= 1;
                sem_post(mutex_shm_sem);
                worker_status[i] = 0;
                continue;
            }
            else    //dealing with sensor data
            {
                //dealing with sensor data
                char id_sensor[32],key[32];
                int value;
                delim = "#";
                ptr = strtok(copy, delim);
                strcpy(id_sensor,ptr);
                ptr = strtok(NULL, delim);
                strcpy(key,ptr);
                ptr = strtok(NULL, "\0");
                value = atoi(ptr);
                sem_wait(mutex_shm_sem);
                if(region->keys_counter == MAX_KEYS)
                {
                    int result = UPDATE_STATS(region->header1, key, value);
                    if(result == 0) //key is previously not in keys array
                    {
                        sem_wait(mutex_log);
                        logging("!LIMIT OF KEYS REACHED!");
                        sem_post(mutex_log);
                        sem_post(mutex_shm_sem);
                        worker_status[i] = 0;
                        continue;
                    }
                }
                else
                {
                    ADD_KEY(region->header1, key, value);
                    sem_wait(mutex_log);
                    logging("WORKER%d: %s KEY INSERTED");
                    sem_post(mutex_log);
                    region->keys_counter += 1;
                }
                sem_post(mutex_shm_sem);
                worker_status[i] = 0;
                continue;
            }

        }

    }

}

void ALERTS_WATCHER(int id)
{
    while(1)
    {

    }
}




int main(int argc, char *argv[])
{
    parent = getpid();
    signal(SIGINT,ctrlc_handler);
    destroy = (int*)malloc(sizeof(int));
    *destroy = 0;
    printf("Destroy set to %d\n",*destroy);

    //creating named pipes
    if ((mkfifo(SENSOR_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) { perror("Cannot create SENSOR_PIPE: ");

    }
    if ((mkfifo(CONSOLE_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) { perror("Cannot create CONSOLE_PIPE: ");

    }

    sem_unlink("THREAD_MUTEX");
    thread_mutex = sem_open("THREAD_MUTEX",O_CREAT|O_EXCL,0700,1);

    //setting up configurations
    setup();


    /*
    int total_childs = N_WORKERS+1;
    worker_pipes = (int**) malloc(N_WORKERS*sizeof(int*));
    worker_status = (int*) malloc(N_WORKERS*sizeof(int));
    process_id = (int*) malloc(total_childs*sizeof(int));
    for(int i = 0; i < N_WORKERS; i++)
    {
        worker_pipes[i] = (int *)malloc(sizeof(int)*2);
        pipe(worker_pipes[i]);
        worker_status[i] = 0;
    }

    //creating semaphore
    sem_unlink("MUTEX_SHM_SEM");
    mutex_shm_sem =sem_open("MUTEX_SHM_SEM",O_CREAT|O_EXCL,0700,1);



    //creating message queue
    msqid = msgget(IPC_PRIVATE,O_CREAT|0700);
    //creating shared memory
    create_shared_memory();
    //creating internal queue
    InternalQueueInitial(&queue);
    */

    //creating dispatcher
    thread_id[0] = 0;
    pthread_create(&threads[0],NULL,DISPATCHER,&thread_id[0]);

    sem_wait(mutex_log);
    logging("THREAD DISPATCHER CREATED");
    sem_post(mutex_log);

    //creating sensor reader
    thread_id[1] = 1;
    pthread_create(&threads[1],NULL,SENSOR_READER,&thread_id[1]);

    sem_wait(mutex_log);
    logging("THREAD SENSOR_READER CREATED");
    sem_post(mutex_log);

    //creating console reader
    thread_id[2] = 2;
    pthread_create(&threads[2],NULL,CONSOLE_READER,&thread_id[2]);

    sem_wait(mutex_log);
    logging("THREAD CONSOLE_READER CREATED");
    sem_post(mutex_log);

    //creating workers and alerts watcher
    //as well as estabilishing unnamed pipes
    // between workers and system manager

    /*
    pid_t pid;
    char message[20];
    for(int i = 0; i < N_WORKERS+1;i++)
    {
        pid = fork();
        if(pid == 0)
        {
            process_id[i] = getpid();
            if(i == 0)
            {
                sem_wait(mutex_log);
                logging("PROCESS ALERTS_WATCHER CREATED");
                sem_post(mutex_log);
                ALERTS_WATCHER(pid);
            }
            else
            {
                sem_wait(mutex_log);
                snprintf(message,sizeof(message),"WORKER %d READY",i);
                logging(message);
                sem_post(mutex_log);
                WORKER(pid,i,worker_pipes[i]);
            }
        }
    }
    */
    //setting up signal
    pause();

}
