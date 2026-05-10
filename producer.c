#include <stdio.h>
#include "share_def.h"


Shm_Data* shareData;
pthread_t RAM_thread, LinkState_thread;
// bool IsContinueLoop = true;
sem_t* sem_available = NULL;
sem_t* sem_filled = NULL;

void handle_exit(int sig)
{
    printf("\n[Warning] Just received Exit signal (%d). Start to cleaning process... \n", sig);
    shareData->IsContinueLoop = false;

    // wake up all semaphores
    sem_post(sem_available);
    sem_post(sem_filled);
}

void* handle_RAM_thread(void* arg)
{
    SensorData sRAMData;
    while((shareData->IsContinueLoop))
    {
        sem_wait(sem_available);
       
        // Wake up then check IMMEDIATELY!
        if (!shareData->IsContinueLoop)
        {
            sem_post(sem_filled);
            break;
        }

        if (shareData->count == QUEUE_SIZE)
        {
            printf("RAM thread: Queue is full!\n");
            // sem_post(sem_available);
            sem_post(sem_filled);   // call 'consumer' get data from queue
            sleep(2);
        }
        
        pthread_mutex_lock(&shareData->shm_lock);

        FILE* ptr_file;
        ptr_file = fopen("/proc/meminfo", "r");

        char name_buf[16];
        unsigned long val_buf;
        while (fscanf(ptr_file, "%s %lu kB", name_buf, &val_buf) == 2)
        {
            if (!strcmp(name_buf, "MemAvailable:"))
            {
                sRAMData.sensorType = SENSOR_RAM;
                sRAMData.sensorVal = val_buf;

                // copy sRAMData into queue of shared mem 
                shareData->q_buffer[shareData->tail].sensorType = sRAMData.sensorType;
                shareData->q_buffer[shareData->tail].sensorVal = sRAMData.sensorVal;

                // count tail of queue
                shareData->tail = (shareData->tail + 1) % QUEUE_SIZE;
                shareData->count++;


                printf("%s %lu kB\n", name_buf, val_buf);
                break;
            }
        }

        fclose(ptr_file);

        pthread_mutex_unlock(&shareData->shm_lock);
        sem_post(sem_filled);

        sleep(2);
    }

    return NULL;
}

void* handle_LinkState_thread(void* arg)
{
    SensorData sLinkStateData;
    while((shareData->IsContinueLoop))
    {
        sem_wait(sem_available);

        // Wake up then check IMMEDIATELY!
        if (!shareData->IsContinueLoop)
        {
            sem_post(sem_filled);
            break;
        }

        if (shareData->count == QUEUE_SIZE)
        {
            printf("Link State thread: Queue is full!\n");
            // sem_post(sem_available);
            sem_post(sem_filled);   // call 'consumer' get data from queue
            sleep(2);
        }
        
        pthread_mutex_lock(&shareData->shm_lock);

        FILE* ptr_file;
        ptr_file = fopen("/sys/class/net/eth0/carrier", "r");

        int c = fgetc(ptr_file);
        if (c != EOF)
        {
            sLinkStateData.sensorType = SENSOR_LINK_STATE;
            sLinkStateData.sensorVal = c;

            // copy sLinkStateData into queue of shared mem 
            shareData->q_buffer[shareData->tail].sensorType = sLinkStateData.sensorType;
            shareData->q_buffer[shareData->tail].sensorVal = sLinkStateData.sensorVal;

            // count tail of queue
            shareData->tail = (shareData->tail + 1) % QUEUE_SIZE;
            shareData->count++;

            printf("Link state: %c\n", c);
        }
        
        fclose(ptr_file);

        pthread_mutex_unlock(&shareData->shm_lock);
        sem_post(sem_filled);

        sleep(2);
    }

    return NULL;
}

int main()
{
    // 1. Create a shared memory object
    int shm_fd;
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    // 2. Config size for this shared memory object
    ftruncate(shm_fd, SHM_SIZE);

    // 3. mapping virtual memory for this shared memory object
    void *ptr = NULL;
    ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (ptr != NULL)
    {
        shareData = (Shm_Data*)ptr;
        shareData->IsContinueLoop = true;

        // Init mutex
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&shareData->shm_lock, &attr);

        // Init semaphore
        sem_available = sem_open(SEM_AVAILABLE_NAME, O_CREAT, 0666, SEM_AVAILABLE_SIZE_INIT);
        sem_filled = sem_open(SEM_FILLED_NAME, O_CREAT, 0666, SEM_FILLED_SIZE_INIT);

        // Sign in take Ctrl+C event
        signal(SIGINT, handle_exit);

        // Init thread
        pthread_create(&RAM_thread, NULL, handle_RAM_thread, NULL);
        pthread_create(&LinkState_thread, NULL, handle_LinkState_thread, NULL);
        

        // Process end
        pthread_join(RAM_thread, NULL);
        pthread_join(LinkState_thread, NULL);

        printf("Start to release all system resource...\n");

        // Semaphrore close & unlink
        sem_close(sem_available);
        sem_close(sem_filled);
        sem_unlink(SEM_AVAILABLE_NAME);
        sem_unlink(SEM_FILLED_NAME);

        // Mutex destroy
        pthread_mutex_destroy(&shareData->shm_lock);

        // Shared memory unlink
        munmap(ptr, SHM_SIZE);  // cancel map virtual memory with the shread memory
        close(shm_fd);          // close file descriptor
        shm_unlink(SHM_NAME);

        printf("Sensor producer process end!\n");
    }
    else
    {
        printf("Create shared memory fail!\n");
    }

    return 88;
}