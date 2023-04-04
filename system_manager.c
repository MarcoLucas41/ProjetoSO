//
// Created by Marco Lucas on 31/03/2023.
//
#include "processes.h"

//setup
int* SETUP();

//threads
void *DISPATCHER(void *id);
void *SENSOR_READER(void *id);
void *CONSOLE_READER(void *id);

//child processes
void WORKER(int id);
void ALERTS_WATCHER(int id);



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

void *DISPATCHER(void *id)
{

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




int main(int argc, char *argv[])
{
    if ((mkfifo(SENSOR_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) { perror("Cannot create SENSOR_PIPE: ");

    }
    if ((mkfifo(CONSOLE_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) { perror("Cannot create CONSOLE_PIPE: ");

    }

    //funcoes de destroir named pipes
    //unlink(SENSOR_PIPE);
    //unlink(SENSOR_PIPE);


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
