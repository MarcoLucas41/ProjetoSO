//
// Created by Marco Lucas on 31/03/2023.
//
#include "processes.h"

int* SETUP();

//threads
void *DISPATCHER(void *t);
void *SENSOR_READER(void *t);
void *CONSOLE_READER(void *t);

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
