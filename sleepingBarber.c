#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_CUSTOMERS 20
#define NUM_CHAIRS 5
#define MAX_QUEUE_SIZE 100

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




typedef struct {
    sem* sema;
} QueueNode;

typedef struct {
    QueueNode* nodes[MAX_QUEUE_SIZE];
    int front;
    int rear;
} Queue;

void initializeQueue(Queue* q) {
    q->front = 0;
    q->rear = -1;
}
int isQueueEmpty(Queue* q) {
    return q->front > q->rear;
}

int isQueueFull(Queue* q) {
    return q->rear == MAX_QUEUE_SIZE - 1;
}

void push(Queue* q, sem* sema) {
    if (isQueueFull(q)) {
        exit(1);
    }
    QueueNode* newNode = (QueueNode*) malloc(sizeof(QueueNode));
    newNode->sema = sema;
    q->nodes[++q->rear] = newNode;
}

sem* pop(Queue* q) {
    if (isQueueEmpty(q)) {
        exit(1);
    }
    QueueNode* node = q->nodes[q->front++];
    sem* sema = node->sema;
    free(node);
    return sema;
}


Queue queue_of_waiting_customers;

sem customers;//semaphore for the number of customers.
sem mutex;
sem barber;//semaphore for the barber.
int num_waiting=0;

void* Barber(){
    while(1){
        wait(&customers);
        wait(&mutex);
        sem * haircut=(pop(&queue_of_waiting_customers));
        num_waiting--;
        signal(&mutex);
        signal(haircut);
        wait(&barber);

    }

}
void* Customer(void* arg){
    int id=*(int *) arg;
    sem waiting_on_self;
    waiting_on_self=*createSemaphore(0);//initialize waiting semaphore to 0

    printf("Customer %d has arrived.\n",id);
    wait(&mutex);
    if(num_waiting==NUM_CHAIRS){
        signal(&mutex);
        printf("Customer %d is leaving the barbershop because of no available chairs.\n", id);
    }
    else{
        num_waiting++;
        printf("Customer %d takes a seat. Number of seats filled: %d. Total number of seats: %d.\n",id,num_waiting,NUM_CHAIRS);
        push(&queue_of_waiting_customers,&waiting_on_self);
        
        signal(&customers);
        signal(&mutex);
        
        wait(&waiting_on_self);
        
        printf("Customer %d is taking a haircut.\n",id);
        sleep(2);
        signal(&barber);
        
        printf("Customer %d is leaving the barbershop after taking haircut.\n", id);
        


    }

}


int main(){
    printf("Program starts\n");
    pthread_t barber_thread;
    pthread_t customer_threads[NUM_CUSTOMERS];
    initializeQueue(&queue_of_waiting_customers);

    customers=*createSemaphore(0);//initialize customers to 0
    barber=*createSemaphore(0); //initialize barber to 0
    mutex=*createSemaphore(1); //initialize mutex to 1
    
    // Create barber thread
    pthread_create(&barber_thread, NULL,&Barber, NULL);

    // Create customer threads
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_create(&customer_threads[i], NULL, &Customer, &i);
        sleep(1);
    }

    // Wait for threads to finish
    
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customer_threads[i], NULL);
    }
    pthread_detach(barber_thread);
    printf("end\n");

}