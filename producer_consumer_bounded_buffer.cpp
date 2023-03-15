#include <stdio.h>
#include <pthread.h>
#include<stdlib.h>
#include<unistd.h>


struct semaphore{
    int v;
    int cnt;
    pthread_mutex_t *mutex;
    pthread_cond_t *condition;
};

typedef struct semaphore sem;

pthread_mutex_t* createMutex(){
    pthread_mutex_t* newMutex=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    return newMutex;
}

pthread_cond_t* createCondition(){
    pthread_cond_t* newCondition=(pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    return newCondition;
}

sem* createSemaphore(int val){
    sem* newSemaphore=(sem*)malloc(sizeof(sem));
    newSemaphore->v=val;
    newSemaphore->cnt=0;
    newSemaphore->mutex=createMutex();
    newSemaphore->condition=createCondition();
    return newSemaphore;
}

void wait(sem* s){
    pthread_mutex_lock(s->mutex);
    s->v--;
    if(s->v<0){
        do{
            pthread_cond_wait(s->condition,s->mutex);
        }while(s->cnt<1);
        s->cnt--;
    }
    pthread_mutex_unlock(s->mutex);
}

void signal(sem* s){
    pthread_mutex_lock(s->mutex);
    s->v++;
    if(s->v<=0){
        s->cnt++;
        pthread_cond_signal(s->condition);
    }
    pthread_mutex_unlock(s->mutex);
}


#define BUFFER_SIZE 50

int buffer[BUFFER_SIZE];

int count = 0;  // the number of items in the buffer

int _index = 0;     // index of the buffer at which item produced/consumed by buffer


sem *mutex_lock;
sem *full;
sem *_empty; 
// declaring semaphores


void *producer(void *arg){
    int item;

    // while (1) {
    for( int i=0 ; i < 50 ; i++){
    // consumer process runs for an infinite time, ideally
        item = rand()%100 + 1;  
        // generate a random item between [1,100]
        wait(mutex_lock);
        // equivalent to pthread_mutex_lock(&mutex_lock());

        while (count == BUFFER_SIZE){
            // pthread_cond_wait(&full, &mutex_lock);
            wait(full);
        }
        
        buffer[_index] = item;

        _index = _index + 1;

        count++;

        printf("Producer %d produced item %d\n",*((int *)arg),item);

        // mutex_lock.signal();
        signal(mutex_lock);
        // equivalent to pthread_mutex_unlock(&mutex);
        signal(_empty);
        // _empty.signal();
        // equivalent to pthread_cond_signal(&empty);
    }
}


void *consumer(void *arg){
    int item;
        // while(1){
        for( int i=0 ; i < 50 ; i++){
        
        // producer process runs for an infinite time, ideally

        // mutex_lock.wait();
        wait(mutex_lock);
        // to acquire the mutex before entering critical section

        while (count == 0){

            wait(_empty);
            // equivalent to pthread_cond_wait(&empty, &mutex);
        }

        if(_index==BUFFER_SIZE){
            
            _index=_index-1;
            // buffer full so accessing the last index, decrease index
        }

        _index = _index - 1;

        item = buffer[_index];

        count--;

        printf("Consumer %d consumed item %d\n",*((int *)arg),item);

        signal(mutex_lock);
        // equivalent to pthread_mutex_unlock(&mutex);

        signal(full);
    }
}


int main(int argc, char **argv)
{

    mutex_lock=createSemaphore(1);
    // initialising semaphores

    full=createSemaphore(0);
    // initialising semaphores

    _empty=createSemaphore(1);
    // initialising semaphores


    pthread_t prod[5], cons[5];
    // create 5 producer and consumer threads

    int num[5] = { 1, 2, 3, 4, 5};
    // array for numbering of produces and consumers

    for( int i=0 ;i < 5;i++ ){

        pthread_create(&prod[i],NULL,producer,(void *)&num[i]);
        // creating producer threads

    }

    for( int i=0 ;i < 5 ;i++ ){

        pthread_create(&cons[i],NULL,consumer,(void *)&num[i]);
        // creating consumer thrads

    }

    for( int i = 0; i < 5; i++ ){

        pthread_join(prod[i],NULL);
        // joining producer threads together

    }
    
    for( int i = 0; i < 5; i++ ){

        pthread_join(cons[i],NULL);
        // joining consuer threads together

    }

    return 0;
}
