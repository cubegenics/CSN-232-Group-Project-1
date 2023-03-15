#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>

//semaphore struct
struct semaphore{
    int value;
    //value represents the value of the semaphore

    int executableCount;
    //executableCount represents the number of threads which 
    //were waiting on the semaphore and can now execute on the semaphore

    pthread_mutex_t *mutex;
    //we require a mutex so that various threads can't simultaneously modify the 
    //various variables of the semaphore
    pthread_cond_t *condition;
    //we require a condition so that a thread waiting on 
    //the semaphore can be blocked
};

typedef struct semaphore sem;
//for ease of coding, we just use typedef so that whenever we wish to write
//"struct semaphore", we can just write "sem" instead.


//methods to create mutex, condition, semaphore
pthread_mutex_t* createMutex(){
    pthread_mutex_t* newMutex=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    //create a new pthread mutex pointer and allocate the necessary space to it
    return newMutex;
}

pthread_cond_t* createCondition(){
    pthread_cond_t* newCondition=(pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    //create a new pthread condition pointer and allocate the necessary space to it
    return newCondition;
}

sem* createSemaphore(int value){
    //initialize the various variables of the semaphore
    sem* newSemaphore=(sem*)malloc(sizeof(sem));
    newSemaphore->value=value;
    newSemaphore->executableCount=0;
    newSemaphore->mutex=createMutex();
    newSemaphore->condition=createCondition();
    return newSemaphore;
}

void wait(sem* cur){//method implementing semaphore wait operation
    pthread_mutex_lock(cur->mutex);
    //the thread needs to gain access to modify variables belonging
    //to the semaphore "cur" 
    cur->value--;
    if(cur->value<0){
        while(cur->executableCount<1)//thread waits while it can't execute
            pthread_cond_wait(cur->condition,cur->mutex);
        cur->executableCount--;//current waiting thread now executes
    }
    pthread_mutex_unlock(cur->mutex);//other threads can now gain access to the semaphore
}

void signal(sem* cur){//method implementing semaphore signal operation
    pthread_mutex_lock(cur->mutex);
    //the thread needs to gain access to modify variables belonging
    //to the semaphore "cur" 
    cur->value++;
    if(cur->value<=0){
        cur->executableCount++;//can execute a thread which was waiting on this semaphore
        pthread_cond_signal(cur->condition);
    }
    pthread_mutex_unlock(cur->mutex);//other threads can now gain access to the semaphore
}