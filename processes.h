//
// Created by Marco Lucas on 30/03/2023.
//
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>	// for O_ constants
#include <semaphore.h>
#include "string.h"
#include <pthread.h>


#ifndef PROJETOSO_PROCESSES_H
#define PROJETOSO_PROCESSES_H


#define MAX_SIZE 256

#define SENSOR_PIPE "sensor_pipe"
#define CONSOLE_PIPE "console_pipe"

int config[5];


typedef struct SensorData
{
    char *key;
    int latest_value;
    int min_value;
    int max_value;
    double avg_values;
    int counter_updates_key;
    struct SensorData *next;
}SensorData;


typedef struct ArrayData
{
    SensorData *first;
    SensorData *last;
}ArrayData;

typedef struct SharedMemory
{
    ArrayData *header;
    char **sensors;

}SharedMemory;

#endif //PROJETOSO_PROCESSES_H
