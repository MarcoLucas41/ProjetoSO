//
// Marco Lucas 2021219146
// Bruno Almeida 2021237081

// Sistemas Operativos
// Licenciatura em Engenharia Informática 2022/2023
//
#include "processes.h"
int message_counter = 0;


void ctrlz_handler(int signo)
{
    printf("Received SIGTSTP signal (Ctrl+Z). Pausing...\n");
    printf("Total messages sent: %d\n", message_counter);

    printf("please press ENTER to continue\n");
    while(getc(stdin)!= '\n');
}

void ctrlc_handler(int signo)
{
    printf("Received SIGINT signal (Ctrl+C). Terminating...\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGTSTP, ctrlz_handler);
    if(argc != 6)
    {
        perror("Wrong number of arguments for sensor!");
        exit(0);
    }

    if(strlen(argv[1]) < 3 || strlen(argv[1]) > 32)
    {
        perror("ID must be between 3 and 32 characters");

    }
    if(strlen(argv[3]) < 3 || strlen(argv[3]) > 32)
    {
        perror("Key must be between 3 and 32 characters");
    }

    char buffer[MAX_LEN_MSG];
    int max,min;
    int num;
    int fd;
    signal(SIGINT, ctrlc_handler);
    printf("OPENING SENSOR PIPE FOR WRITING\n");
    if ( ( fd = open(SENSOR_PIPE, O_WRONLY) ) < 0)
    {
        perror("Cannot open SENSOR PIPE for WRITING: ");
        exit(0);
    }
    printf("AFTER OPENING SENSOR PIPE FOR WRITING\n");
    min = atoi(argv[4]);
    max = atoi(argv[5]);

    while(1)
    {
        num = rand()%(max-min) + min;
        printf("%d\n",num);
        snprintf(buffer,MAX_LEN_MSG,"%s#%s#%d",argv[1],argv[3],num);
        write(fd,buffer,MAX_LEN_MSG);
        sleep(atoi(argv[2]));
        message_counter +=1;
    }
}

