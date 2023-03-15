#include<sys/types.h>
#include<sys/ipc.h>
#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>

typedef pthread_mutex_t Mutex;
typedef pthread_cond_t Cond;

typedef struct{
    int value,checkloop;
    Mutex* mutex;
    Cond* cond;
}semaphore;



semaphore* semaphore_init(int value){
    semaphore* sem = (semaphore*)malloc(sizeof(semaphore)); //allocating space to the semaphore
    sem->value = value;             //initializing semaphore's value
    sem->checkloop = 0;             //starting value for checkloop  for waiting on condition variable
    sem->mutex = (Mutex*)malloc(sizeof(Mutex));        //allocating space to the mutex
    pthread_mutex_init(sem->mutex,NULL);    //initializing mutex
    sem->cond = (Cond*)malloc(sizeof(Cond));            //allocating space to the condition variable
    pthread_cond_init(sem->cond,NULL);      //initializing condition variable
    return sem;
}

void semaphore_wait(semaphore* sem){
    pthread_mutex_lock(sem->mutex);     //locking the mutex so that no other thread can modify semaphore's value 
    sem->value--;                       
    if(sem->value<0){                  //if some process is already in C.S
        do{     
            pthread_cond_wait(sem->cond,sem->mutex);        //looping until some signalled by other thread
        }while(sem->checkloop<1);                           //after incrementing checkloop's value
        sem->checkloop--;                           //allowing only 1 process at a time to come out of wiating loop while
    }                                               //locking the mutex
    pthread_mutex_unlock(sem->mutex);       // unlocking mutex
}

void semaphore_signal(semaphore* sem){
    pthread_mutex_lock(sem->mutex);     //locking the mutex so that no other thread can modify semaphore's value
    sem->value++;                       
    if(sem->value<=0){
        sem->checkloop++;           //incrementing checkloop to call one process out of waiting
        pthread_cond_signal(sem->cond);         //by signalling the condition variable
    }
    pthread_mutex_unlock(sem->mutex);        // unlocking mutex
}

semaphore* rd_mutex,*turn,*resource;    //Creating references to the semaphores required
int shared_var = 0;                     //initializing value of shared variable
int rd_cnt = 0;                         //initial number of readers
void *writer(void *id)                  //writer function
{   
    printf("Writer %d wants to write\n",(*((int *)id)));
    semaphore_wait(turn);       //waiting for its turn for execution
    
    semaphore_wait(resource);   //waiting if at present some reader or writer is in C.S
    
    printf("Writer %d is writing\n",(*((int *)id))); 
    shared_var++;   
    sleep(3);       //time for writing
    printf("Writer %d modified shared variable to %d\n",(*((int *)id)),shared_var);
    // sleep(3);
    semaphore_signal(turn); //Signalling the next reader or writer process in turn queue to wakeup
    semaphore_signal(resource); //releasing the resource for the other reader or writer process
    printf("Writer %d exited\n",(*((int *)id)));
}
void *reader(void *id)
{   
    printf("Reader %d wants to read\n",(*((int *)id)));
    semaphore_wait(turn);       //waiting for its turn for execution
    semaphore_wait(rd_mutex);   //ensuring only one reader process gets the chance to update read count variable
    rd_cnt++;                   //updating read count variable
    printf("Value of rd_cnt %d\n",rd_cnt);  
    if(rd_cnt==1){
        semaphore_wait(resource);       //first reader will ask for resource access for readers
    }

    semaphore_signal(turn);             //Signalling the next reader or writer process in turn queue to wakeup 
                                        //if reader process it will wait on rd_mutex and if writer it will wait on
                                        //resource
    semaphore_signal(rd_mutex);         //releasing access for updation of read count
    
    printf("Reader %d is reading\n",(*((int *)id)));

    sleep(2);       //time for reading

    printf("Reader %d: the value of shared variable %d \n",(*((int *)id)),shared_var);
    
    
    printf("Reader %d wants to exit\n",(*((int *)id)));
    semaphore_wait(rd_mutex);           //ensuring only one read process gets the chance to update read count variable
    rd_cnt--;                           //updating read count variable
    
    printf("Reader %d exited\n",(*((int *)id)));
    printf("Value of rd_cnt %d\n",rd_cnt);
    if(rd_cnt==0){
        semaphore_signal(resource);     //freeing the resources for writers to write if some 
                                        //writer process(es) is waiting on resource after being removed from turn semaphore
        // printf("Now a writer can write if present already for its turn\n");
    }
    semaphore_signal(rd_mutex);         //releasing access for updation of read count
    
}

int main()
{   
    int n;
    printf("Input the total number of threads to be created\n");
    scanf("%d",&n);
    pthread_t threadarr[n];     
    int id[n];              //id's of threads
    int readnum = 0,writenum = 0;   //initial number of readers and writers
    srand((unsigned int)time(NULL));
    rd_mutex = semaphore_init(1);       //Initializing the values of the three semaphores required in the solution
    turn = semaphore_init(1);       
    resource = semaphore_init(1);
    
   for(int i = 0; i < n; i++) {
         if(rand()%2){                  //randomly generating reader and writer threads
         id[i] = ++writenum;
         pthread_create(&threadarr[i], NULL, (void *)writer, (void *)&id[i]);     //creating a writer thread
         }
         else{
         id[i] = ++readnum;
         pthread_create(&threadarr[i], NULL, (void *)reader, (void *)&id[i]);   //creating a reader thread
         }
         
         sleep(1);          //time for creation of threads
    }
    
    for(int i = 0; i < n; i++) {
       pthread_join(threadarr[i], NULL);        //waiting till executiom of all threads
    }
   
   
    printf("All threads successfully executed\n");
    return 0;
    
}

 