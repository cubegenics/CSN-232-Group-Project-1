#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

typedef struct{
    int value;
    int wakeup;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
}Semaphore;

Semaphore* sem_init(int value){
    Semaphore* sem = (Semaphore*)malloc(sizeof(Semaphore)); //Allocate space for the semaphore struct
    sem->value = value; //Initialise the value to the parameter passed
    sem->wakeup = 0;
    (sem->mutex) = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)); //Allocate space for mutex
    (sem->cond) = (pthread_cond_t*)malloc(sizeof(pthread_cond_t)); //Allocate space for cond
    return sem;
}

void sem_wait(Semaphore *sem){
    pthread_mutex_lock(sem->mutex); //Lock the semaphore mutex to ensure mutual-exclusion in the usage of semaphore
    sem->value = sem->value - 1; //Reduce the value of semaphore by 1
    if(sem->value < 0){ //If the semaphore cannot be acquired right now
        do{
            pthread_cond_wait(sem->cond, sem->mutex); //Wait for cond and release mutex in the meantime
        }
        while(sem->wakeup < 1); //Keep waiting unless wakeup >= 1
        sem->wakeup = sem->wakeup - 1; //Thread which is released from loop decreases value of wakeup to ensure that only one thread executes when cond is signalled
    }
    pthread_mutex_unlock(sem->mutex); //Unlock the semaphore mutex
}

void sem_wait_mutex(Semaphore *sem, char s[],int x){
    //Same code as above, just used specifically for mutex in code to print the Producer/Consumer number which tries to acquire mutex to verify FIFO order to eliminate STARVATION
    pthread_mutex_lock(sem->mutex);
    sem->value = sem->value - 1;
    printf("%s %d wants to execute!\n", s, x);
    if(sem->value < 0){
        do{
            pthread_cond_wait(sem->cond, sem->mutex);
        }
        while(sem->wakeup < 1);
        sem->wakeup = sem->wakeup - 1;
    }
    pthread_mutex_unlock(sem->mutex);
}

void sem_post(Semaphore* sem){
    pthread_mutex_lock(sem->mutex); //Lock semaphore mutex
    sem->value = sem->value + 1; //Increment the value of semaphore
    if(sem->value <= 0){ //If any thread was waiting for the signal
        sem->wakeup = sem->wakeup + 1; //Increment wakeup by 1
        pthread_cond_signal(sem->cond); //Signal cond
    }
    pthread_mutex_unlock(sem->mutex); //Unlock the semaphore mutex
}

#define INF 100000

int buffer[INF];
int in=0,out=0,count=20;

Semaphore* cons;
Semaphore* mutex;

void *Producer_f(void *arg){
    char s[] = "Producer";
    while (count>0){
        sem_wait_mutex(mutex,s,(int) arg); //Producer thread tries to acquire mutex
        int value = rand()%999; //Random value of item generated
        buffer[in] = value; //Item is written in buffer
        printf("Producer %d : Produced %d\n",(int) arg, value);
        in = in + 1; //The index of buffer to be produced next is incremented
        count = count - 1; //COMMENT THIS LINE FOR INFINITE GENERATION OF ITEMS IN BUFFER, OTHERWISE ONLY 20 ITEMS ARE PRODUCED FOR EASE OF UNDERSTANDING THE OUTPUT GENERATED
        sem_post(cons); //Signal the consumers that there exists an item in the buffer that is yet to be consumed
        sem_post(mutex); //Release mutex
        sleep(1);
    }
}

void *Consumer_f(void *arg){
    char s[] = "Consumer";
    while (true){
        if(count==0){ //If producers have stopped producing, signal cons in order to make consumers exit
            sem_post(cons);
        }
        sem_wait(cons); //Wait for an item to exist in the buffer that is not consumed yet
        sem_wait_mutex(mutex,s,(int) arg); //Consumer thread tries to acquire mutex
        if(count==0 & in==out){//If producers have stopped producing and all items have been consumed
            printf("Consumer %d exitting!\n", (int) arg);
            sem_post(mutex);
            sem_post(cons);
            break;
        }
        int item = buffer[out]; //Item is read from buffer
        printf("Consumer %d : Consumed %d\n",(int) arg,item);
        out = out + 1; //The index of buffer to be consumed next is incremented
        sem_post(mutex); //Release mutex
        sleep(1);
    }
}

int main(){
    cons = sem_init(0); //Initialise cons to be 0 as there are no items in the buffer initially which can be consumed
    mutex = sem_init(1); //Initialise mutex to be 1
    pthread_t producer[5], consumer[10];
    //Create 5 producer threads and 10 consumer threads
    for(int i=0;i<10;i++){
        pthread_create(&consumer[i],NULL,Consumer_f,(int*) (i+1));
    }
    for(int i=0;i<5;i++){
        pthread_create(&producer[i],NULL,Producer_f,(int*) (i+1));
    }
    for(int i=0;i<5;i++){
        pthread_join(producer[i],NULL);
    }
    for(int i=0;i<10;i++){
        pthread_join(consumer[i],NULL);
    }
    return 0;
}