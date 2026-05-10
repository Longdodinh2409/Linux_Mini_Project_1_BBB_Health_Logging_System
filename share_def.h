#ifndef SHARE_DEF_H
#define SHARE_DEF_H

#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// Signals (for Exit by Ctrl+C)
#include <signal.h>

// Semaphore
#include <semaphore.h>

// Shared memory
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>
#include <unistd.h>

#define QUEUE_SIZE  (5)

#define SHM_SIZE    (4096)
#define SHM_NAME    ("bbb_health_shm")

#define SEM_AVAILABLE_NAME   ("sema4_available")
#define SEM_AVAILABLE_SIZE_INIT  (QUEUE_SIZE)

#define SEM_FILLED_NAME    ("sema4_filled")
#define SEM_FILLED_SIZE_INIT   (0)

typedef enum {
    SENSOR_RAM,
    SENSOR_LINK_STATE
} SensorTypeData;

typedef struct {
    SensorTypeData sensorType;
    unsigned long sensorVal;
} SensorData;

// Shared memory object's structure

typedef struct {
    SensorData q_buffer[QUEUE_SIZE];
    int head;                           // index for Consumer
    int tail;                           // index for Producer
    int count;                          // number of available element

    pthread_mutex_t shm_lock;

    bool IsContinueLoop;
} Shm_Data;

#endif