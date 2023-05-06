//
// Marco Lucas 2021219146
// Bruno Almeida 2021237081

// Sistemas Operativos
// Licenciatura em Engenharia Informática 2022/2023
//
#include "processes.h"

void menu();

char buffer[MAX_LEN_MSG];
int fd;

//user console SIGINT
void ctrlc_handler(int signal_num)
{
    printf("Received SIGINT signal (Ctrl+C). Cleaning up resources and exiting...\n");
    // Cleanup resources here...
    close(fd);
    exit(0);
}


void menu()
{
    while(1)
    {
        fgets(buffer, MAX_LEN_MSG, stdin);
        if ((strlen(buffer) > 0) && (buffer[strlen(buffer) - 1] == '\n'))
            buffer[strlen(buffer) - 1] = '\0';
        write(fd,buffer,sizeof(buffer));
    }
}




int main(int argc, char *argv[])
{
    signal(SIGINT,ctrlc_handler);
    if(argc != 2)
    {
        printf("Wrong number of arguments for user console.\n");
        exit(1);
    }
    printf("Starting CONSOLE_READER FOR WRITING\n");
    if ( ( fd = open(CONSOLE_PIPE, O_WRONLY) ) < 0)
    {
        perror("Cannot open CONSOLE PIPE for WRITING: ");
        exit(0);
    }
    printf("after OPENING CONSOLE PIPE FOR WRITING\n");
    menu();
    exit(0);

}


