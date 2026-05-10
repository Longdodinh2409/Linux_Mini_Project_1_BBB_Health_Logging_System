#include <stdio.h>
#include <time.h>
#include "share_def.h"

Shm_Data* shareData;
// bool IsContinueLoop = true;
sem_t* sem_available = NULL;
sem_t* sem_filled = NULL;
SensorData sSensorData;

void handle_exit(int sig)
{
    printf("\n[Warning] Just received Exit signal (%d). Start to cleaning process... \n", sig);
    shareData->IsContinueLoop = false;

    // wake up all semaphores
    sem_post(sem_available);
    sem_post(sem_filled);
}

int main()
{
    // Sign in take Ctrl+C event
    signal(SIGINT, handle_exit);

    // 1. Create a shared memory object
    int shm_fd;
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        printf("Please run Producer process first!\n");
        return 1;
    }

    // 2. [No need] Config size for this shared memory object

    // 3. mapping virtual memory for this shared memory object
    void *ptr = NULL;
    ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (ptr != NULL)
    {
        // Init semaphore
        sem_available = sem_open(SEM_AVAILABLE_NAME, 0);
        sem_filled = sem_open(SEM_FILLED_NAME, 0);

        shareData = (Shm_Data*)ptr;

        // Open log file
        FILE* log_fp = fopen("BBB_health_log.txt", "w");
        if (log_fp == NULL)
        {
            printf("Fail at open & create BBB_health_log.txt file\n");
            return 1;
        }
        // else // normally
        {
            printf("Consumer is running and wating for data... Press Ctrl+C to exit!\n");
            printf("----------------------------------------------------------------\n");
        }

        while(shareData->IsContinueLoop)
        {
            sem_wait(sem_filled);
            
            // Wake up then check IMMEDIATELY!
            if (!shareData->IsContinueLoop)
            {
                for (int i = 0; i < NUM_PRODUCER_THREADS; i++)
                {
                    sem_post(sem_available);
                }
                break;
            }

            pthread_mutex_lock(&shareData->shm_lock);
            
            // Main task
            // get data
            sSensorData = shareData->q_buffer[shareData->head];
            // count head of queue
            shareData->head = (shareData->head + 1) % QUEUE_SIZE;
            shareData->count--;

            // get local time
            time_t now_t = time(NULL);
            struct tm* p_time = localtime(&now_t);
            char time_string[64];
            strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", p_time);

            if (sSensorData.sensorType == SENSOR_RAM)
            {
                printf("[%s] RAM available: %lu kB\n", time_string, sSensorData.sensorVal);
                fprintf(log_fp, "[%s] RAM available: %lu kB\n", time_string, sSensorData.sensorVal);
            }
            else if (sSensorData.sensorType == SENSOR_LINK_STATE)
            {
                printf("[%s] Link State now: %c\n", time_string, sSensorData.sensorVal);
                fprintf(log_fp, "[%s] Link state: %c\n",  time_string, (char)(sSensorData.sensorVal));
            }
            fflush(stdout);  // Buộc in buffer
            
            pthread_mutex_unlock(&shareData->shm_lock);
            sem_post(sem_available);
        }
        
        printf("Start to release all system resource...\n");
        // close stream to BBB_health_log.txt
        fclose(log_fp);
        // cancel map virtual memory with the shread memory
        munmap(ptr, SHM_SIZE);
         // close file descriptor
        close(shm_fd);         
        // close semaphores
        sem_close(sem_available);
        sem_close(sem_filled);

        printf("Consumer process end!\n");
    }
    else
    {
        printf("Create shared memory fail!\n");
    }

    return 0;
}