//
// Created by Marco Lucas on 31/03/2023.
//
#include "processes.h"
void menu(int fd);
#define MAX_SIZE 256

void menu(int fd)
{
    char *buffer;
    char *temp;
    buffer = (char*)malloc(MAX_SIZE * sizeof(char));
    while(1)
    {
        temp = (char*)malloc(MAX_SIZE * sizeof(char));
        fgets(temp, MAX_SIZE, stdin);
        strcpy(buffer,temp);

        if ((strlen(temp) > 0) && (temp[strlen(temp) - 1] == '\n') )
            temp[strlen(temp) - 1] = '\0';


        char *delim = " ";
        char *ptr = strtok(temp, delim);

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
    }
    menu(fd);
    exit(0);

}


