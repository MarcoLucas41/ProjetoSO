//
// Created by Marco Lucas on 31/03/2023.
//
#include "processes.h"

//handler
void ctrlc_handler(int signal_num);

//setup
void setup();
void create_shared_memory();

//threads
void *DISPATCHER(void *id);
void *SENSOR_READER(void *id);
void *CONSOLE_READER(void *id);

//child processes
void WORKER(int id);
void ALERTS_WATCHER(int id);

//helper functions
int check_sensor();
int add_sensor();

int QUEUE_SZ,N_WORKERS,MAX_KEYS,MAX_SENSORS,MAX_ALERTS;


int shmid;
char **sensors;

//shared memory region
SharedMemory *region;


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
    fclose(f);
}

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
    while(1)
    {
        read(fd, &buffer, sizeof(buffer));
    }

}

void *SENSOR_READER(void *id)
{

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
    QUEUE_SZ = config[0];
    N_WORKERS = config[1];
    MAX_KEYS = config[2];
    MAX_SENSORS= config[3];
    MAX_ALERTS = config[4];


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



    int N_WORKERS = config[1];
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
    pthread_create(&threads[0],NULL,SENSOR_READER,&thread_id[1]);

    thread_id[2] = 2;
    pthread_create(&threads[0],NULL,CONSOLE_READER,&thread_id[2]);

    for(int i = 0; i < 3; i++)
    {
        pthread_join(threads[i],NULL);
    }

    //signal(SIGINT,funcao de limpeza do sistema);

    //struct sigaction novo;
    //novo.sa_handler = ctrlc_handler;
    //sigfillset(&novo.sa_mask);
    //novo.sa_flags = 0;
    //sigaction(SIGINT,&novo,NULL);

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
