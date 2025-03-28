//
// Marco Lucas 2021219146
// Bruno Almeida 2021237081

// Sistemas Operativos
// Licenciatura em Engenharia Informática 2022/2023
//
#include "processes.h"

void menu();

char buffer[MAX_LEN_MSG];
InternalQueueBlock block;
int console_id;
int fd;
int msqid;
pthread_t thread[2];
long thread_id[2];
int *destroy;

//user console SIGINT
void ctrlc_handler(int signal_num)
{
    printf("Received SIGINT signal (Ctrl+C). Cleaning up resources and exiting...\n");
    // Cleanup resources here...
    *destroy = 1;
    pthread_join(thread[0],NULL);
    pthread_join(thread[1],NULL);
    msgctl(msqid,IPC_RMID,NULL);
    free(destroy);
    close(fd);
    exit(0);
}

void *MSQ_READER(void *id)
{
    MessageStruct mq;
    msqid = msgget(1234,0);
    while(*destroy == 0)
    {
        msgrcv(msqid,&mq,sizeof(MessageStruct)-sizeof(long),console_id,0);
        //msgrcv(msqid,&mq,sizeof(MessageStruct)-sizeof(long),0,0);
        if(strcmp(mq.message,"") != 0 )
        {
            printf("%s",mq.message);
            strcpy(mq.message,"");
        }
    }
    pthread_exit(NULL);
}

void *ALERT_READER(void *id)
{
    MessageStruct mq;
    msqid = msgget(1234,0);
    while(*destroy == 0)
    {
        msgrcv(msqid,&mq,sizeof(MessageStruct)-sizeof(long),9999,0);
        if(strcmp(mq.message,"") != 0 )
        {
            printf("%s",mq.message);
            strcpy(mq.message,"");
        }
    }
    pthread_exit(NULL);
}

void menu()
{
    while(1)
    {
        fgets(buffer, MAX_LEN_MSG, stdin);
        if ((strlen(buffer) > 0) && (buffer[strlen(buffer) - 1] == '\n'))
            buffer[strlen(buffer) - 1] = '\0';
        strcpy(block.command,buffer);
        write(fd,&block,sizeof(block));
    }
}




int main(int argc, char *argv[])
{
    msqid = msgget(123,0);
    signal(SIGINT,ctrlc_handler);
    destroy = (int*)malloc(sizeof(int));
    *destroy = 0;
    if(argc != 2)
    {
        printf("Wrong number of arguments for user console\n");
        exit(1);
    }
    strcpy(block.id,argv[1]);
    console_id = atoi(argv[1]);
    if ( ( fd = open(CONSOLE_PIPE, O_WRONLY) ) < 0)
    {
        error("Cannot open CONSOLE PIPE for WRITING");
        exit(1);
    }
    if(pthread_create(&thread[0],NULL,MSQ_READER,&thread_id[0]) != 0)
    {
        printf("Error in creating message queue thread\n");
        exit(1);
    }
    if(pthread_create(&thread[1],NULL,ALERT_READER,&thread_id[1]) != 0)
    {
        printf("Error in creating message queue thread\n");
        exit(1);
    }

    menu();
    exit(0);

}


