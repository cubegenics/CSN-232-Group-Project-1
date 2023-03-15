#include<sys/types.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct{
    int value, wakeup;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
}Semaphore;

pthread_mutex_t* make_mutex(){  // function to make a mutex variable
    pthread_mutex_t* mutex=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex,NULL);
    return(mutex);
}

pthread_cond_t* make_cond(){    // function to make a condition variable 
    pthread_cond_t* cond=(pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(cond,NULL);
    return(cond);
}

Semaphore* make_semaphore(int value){   // function to make and initialize semaphore
    Semaphore* semaphore=(Semaphore*)malloc(sizeof(Semaphore)); // allocating size of semaphore
    semaphore->value=value; // initializing its value
    semaphore->wakeup=0;
    semaphore->mutex=make_mutex();  
    semaphore->cond=make_cond();
    return(semaphore);
}

void sem_wait(Semaphore *semaphore){
    pthread_mutex_lock(semaphore->mutex);
    semaphore->value--;
    if(semaphore->value < 0){
        do{
            pthread_cond_wait(semaphore->cond,semaphore->mutex);
        }
        while (semaphore->wakeup < 1);
        semaphore->wakeup--;
    }
    pthread_mutex_unlock(semaphore->mutex);
}

void sem_signal(Semaphore* semaphore){
    pthread_mutex_lock(semaphore->mutex);
    semaphore->value++;
    if(semaphore->value<=0){
        semaphore->wakeup++;
        pthread_cond_signal(semaphore->cond);
    }
    pthread_mutex_unlock(semaphore->mutex);
}


#define N 5 // number of philosophers

Semaphore *chopsticks[N]; // semaphores for each philosopher
Semaphore *mutex1; // semaphore for mutually exclusive picking and putting down chopsticks

int state[N] = {0}; // 0 = thinking, 1 = hungry, 2 = eating
// `state` array is used to keep track of the state of each philosopher

int blocks[N]={0}; // to count the no. of times a philosopher was blocked; used to prevent starvation

void test(int id) {
    if ((state[id] == 1 || state[id] == 2) && state[(id + 1) % N] != 2 && state[(id + N - 1) % N] != 2) {
        state[id] = 2; // philosopher can start eating if it is hungry and its neighbours are not eating
        blocks[id]=0;
        sem_signal(chopsticks[id]); // signal that the chopsticks are available
    }
    else if(state[id] == 1){
        blocks[id]++;
        if(blocks[id] == 10) state[id] == 2;
        // if a philosopher is blocked consecutively enough no of times, it is set to eating so that neither of its neighbours can eat again till this philosopher had gotten its chance to eat. This prevents starvation
    }
}

void pickup_chopsticks(int id) {
    sem_wait(mutex1);

    state[id] = 1; // philosopher is hungry
    test(id); // try to pick up both chopsticks

    sem_signal(mutex1);
    sem_wait(chopsticks[id]); // block if chopsticks were not acquired
}

void return_chopsticks(int id) {
    sem_wait(mutex1);

    state[id] = 0; // philosopher is done eating
    test((id + N - 1) % N); // check if left neighbor can eat now
    test((id + 1) % N); // check if right neighbor can eat now

    sem_signal(mutex1);
}

void *philosopher(void *arg) {
    int id = *(int *)arg;
    int x=0;
    while(1) {
        if(x==0) printf("Philosopher %d is thinking\n", id);
        sleep(1); // simulate thinking

        printf("Philosopher %d is hungry\n", id);
        pickup_chopsticks(id); // pick up both chopsticks

        printf("Philosopher %d is eating\n", id);
        sleep(1); // simulate eating

        printf("Philosopher %d finished eating and started thinking\n", id);
        return_chopsticks(id); // return both chopsticks
        x++;
    }
    return NULL;
}

int main() {
    pthread_t philosophers[N];
    int ids[N];

    // Initialize semaphores
for(int i=0;i<N;i++){
    chopsticks[i]=make_semaphore(0);
}
mutex1=make_semaphore(1);

    // Create philosopher threads
    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher, &ids[i]);
    }

    // Wait for philosopher threads to finish
    for (int i = 0; i < N; i++) {
        pthread_join(philosophers[i], NULL);
    }

    return 0;
}
