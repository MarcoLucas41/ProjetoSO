//
// Created by Marco Lucas on 31/03/2023.
//
#include "processes.h"
int main(int argc, char *argv[])
{
    if(argc != 6)
    {
        perror("Wrong number of arguments for sensor!");
        exit(0);
    }

    char buffer[20];
    int max,min;
    int num;
    int fd;

    if ((fd = open(SENSOR_PIPE, O_WRONLY)) < 0)
    {
        perror("Cannot open pipe for writing: ");
    }

    min = atoi(argv[4]);
    max = atoi(argv[5]);

    while(1)
    {
        num = rand()%(max-min) + min;
        snprintf(buffer,sizeof(buffer),"%s#%s#%d",argv[1],argv[3],num);
        write(fd,buffer,sizeof(buffer));
        sleep(atoi(argv[2]));
    }
}

