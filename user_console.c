//
// Created by Marco Lucas on 31/03/2023.
//
#include "processes.h"
void menu(int fd);
#define MAX_SIZE 256

char *buffer;
char *temp;
char *ptr;
int fd;

//user console SIGINT
void ctrlc_handler(int signal_num)
{

    printf("Received SIGINT signal (Ctrl+C). Cleaning up resources and exiting...\n");


    // Cleanup resources here...
    free(ptr);
    free(temp);
    free(buffer);
    close(fd);
    // ...

    exit(0);
}


void menu(int fd)
{
    buffer = (char*)malloc(MAX_SIZE * sizeof(char));
    while(1)
    {
        temp = (char*)malloc(MAX_SIZE * sizeof(char));
        fgets(temp, MAX_SIZE, stdin);
        strcpy(buffer,temp);

        if ((strlen(temp) > 0) && (temp[strlen(temp) - 1] == '\n') )
            temp[strlen(temp) - 1] = '\0';


        char *delim = " ";
        ptr = strtok(temp, delim);

        if(strcmp(ptr,"exit") == 0)
        {
            free(ptr);
            free(temp);
            free(buffer);
            close(fd);
            printf("Console turning off...\n");
            break;
        }
        else if( strcmp(buffer,"add_alert") == 0 )
        {

        }
        else if(strcmp(buffer,"remove_alert") == 0)
        {

        }
        else if( strcmp(ptr,"list_alerts") == 0 || strcmp(ptr,"stats") == 0 ||
                 strcmp(ptr,"reset") == 0 || strcmp(ptr,"sensors") == 0)
        {
            write(fd,buffer,sizeof(buffer));
        }

        else
        {
            free(temp);
            free(ptr);
            printf("Command not found\n");
            continue;
        }
        write(fd,buffer,sizeof(buffer));
    }
    exit(0);
}




int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        perror("Wrong number of arguments for user console!");
        exit(0);
    }
    int fd;
    if ((fd = open(CONSOLE_PIPE, O_WRONLY)) < 0)
    {
        perror("Cannot open pipe for writing: ");
        exit(0);
    }
    signal(SIGINT,ctrlc_handler);
    menu(fd);
    exit(0);

}


