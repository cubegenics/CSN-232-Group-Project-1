#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

#define bufferSize 10  // Size of the cyclic buffer
#define NumProduce 5
#define NumConsume 5

int buffer[bufferSize];  // The buffer in which the items are stored
int in=0, out=0;  // The position from where to write and read respectively
int packed=0, vacant=bufferSize;  // packed tell how much buffer is full. vacant tell how much buffer is empty

typedef pthread_mutex_t Mutex;
typedef pthread_cond_t Cond;

typedef struct{
    int value,trigger;
    Mutex *mutex;
    Cond *cond;
}Semaphore;

Semaphore* mutex, *full, *empty;  // Semaphores for mutual exclusion and to prevent starvation & deadlock

Mutex *createMutex(void){  // Function to create a mutex variable
    int a;
    Mutex *mutex = (Mutex*)malloc(sizeof(Mutex));
    a = pthread_mutex_init(mutex,NULL);
    return (mutex);
}

Cond *createCond(void){  // Function to create a condition variable for mutex
    int a;
    Cond *cond = (Cond*)malloc(sizeof(Cond));
    a = pthread_cond_init(cond,NULL);
    return (cond);
}

Semaphore *createSemaphore(int v){  // Function to create semaphore for the critical section
    Semaphore *semaphore = (Semaphore*)malloc(sizeof(Semaphore));
    semaphore->value = v;
    semaphore->trigger = 0;
    semaphore->mutex = createMutex();
    semaphore->cond = createCond();
    return (semaphore);
}

void semaphoreWait(Semaphore *semaphore){  // Function to check the waiting condtion on a semaphore 
    pthread_mutex_lock(semaphore->mutex);
    semaphore->value--;
    if(semaphore->value<0){
        do{
            pthread_cond_wait(semaphore->cond,semaphore->mutex);
        }while(semaphore->trigger<1);
        semaphore->trigger--;
    }
    pthread_mutex_unlock(semaphore->mutex);
}

void semaphoreSignal(Semaphore *semaphore){  // Function to signal the semaphore to resume the waiting process
    pthread_mutex_lock(semaphore->mutex);
    semaphore->value++;
    if(semaphore->value<=0){
        semaphore->trigger++;
        pthread_cond_signal(semaphore->cond);
    }
    pthread_mutex_unlock(semaphore->mutex);
}

void *producer(void *arg){
    int item;
    int i;
    int id = *(int*)arg;
    while(1){
        if(packed==bufferSize) printf("Buffer is full. Waiting for consumer to consume.\n");
        item = rand()%100; // Generate a random item to produce
        printf("Producer %d wants to produce\n", id);
        semaphoreWait(empty); // Wait for an empty slot in the buffer
        semaphoreWait(mutex); // Lock the buffer
        buffer[in] = item;
        printf("Producer %d produced item %d at position %d\n", id, item, in);
        in = (in+1)%bufferSize; // Increment the in pointer
        packed = packed+1;
        if(vacant>0) vacant = vacant-1;
        semaphoreSignal(mutex); // Unlock the buffer
        semaphoreSignal(full); // Signal that a new item is available
        usleep(1+rand()%1000000); // Sleep for a random amount of time
    }
    // pthread_exit(NULL);
}

void *consumer(void *arg){
    int item;
    int i;
    int id = *(int*)arg;
    while(1){
        if(vacant==bufferSize) printf("Buffer is empty. Waiting for producer to produce.\n");
        printf("Consumer %d wants to consume\n", id);
        semaphoreWait(full); // Wait for a full slot in the buffer
        semaphoreWait(mutex); // Lock the buffer
        item = buffer[out];
        printf("Consumer %d consumed item %d from position %d\n", id, item, out);
        out = (out+1)%bufferSize; // Increment the out pointer
        vacant = vacant+1;
        if(packed>0) packed = packed-1;
        semaphoreSignal(mutex); // Unlock the buffer
        semaphoreSignal(empty); // Signal that an empty slot is available
        usleep(1+rand() % 1000000); // Sleep for a random amount of time
    }
    // pthread_exit(NULL);
}

int main(){
    srand(time(NULL));
    pthread_t producers[NumProduce];
    pthread_t consumers[NumConsume];
    int producerId[NumProduce];
    int consumerId[NumConsume];

    mutex = createSemaphore(1); // Initialize the mutex semaphore to 1 
    full = createSemaphore(0); // Initialize the full semaphore to 0
    empty = createSemaphore(bufferSize); // Initialize the empty semaphore to bufferSize

    for(int i=0;i<NumProduce;i++){  // Creating a given number of producer threads
        producerId[i] = i+1;
        pthread_create(&producers[i], NULL, producer, &producerId[i]);
    }

    for(int i=0;i<NumConsume;i++){  // Creating a given number of consumer threads
        consumerId[i] = i+1;
        pthread_create(&consumers[i], NULL, consumer, &consumerId[i]);
    }

    for(int i=0;i<NumProduce;i++){  // Waiting for the producer threads to finish execution
        pthread_join(producers[i],NULL);
    }

    for(int i=0;i<NumConsume;i++){  // Waiting for the consumer threads to finish execution
        pthread_join(consumers[i],NULL);
    }
    return 0;
}
// Please run the code in Ubuntu