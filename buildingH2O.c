/*
Name: Atharv Chhabra
Enrollment Number: 21118025
*/

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include"sem.h"


sem *waitingHydrogen, *waitingOxygen, *mutex;
//semaphores to account for the waiting hydrogen and oxygen threads

//mutex to gain access to modify the number of hydrogen and oxygen threads

int oxygenCount=0, hydrogenCount=0;
//number of oxygen and hydrogen threads


void createWater(){
    //the thread calling this method creates a "virtual bond" with the other
    //valid available threads so as to contribute towards the formation
    //of the H2O molecule
    static int i=0;
    //variable to denote how many threads have called createWater() 
    //it's static as it's shared by all threads and doesn't need to be 
    //initialized multiple times
    i++;
    if(i%3==0){
        //this section is only entered when two hydrogen threads and one oxygen thread have called
        //createWater, implying that an H2O molecule can now be formed
        printf("Molecule number %d created\n\n", i/3);
    }
    sleep(2);
    //sleep is called so that we can actually the multiple H2O molecules being formed
    //as valid number of threads are created
}

void* oxygenFunction(){
    //function called by oxygen threads
    while(1){
        wait(mutex);
        //gain access to mutex so as to modify 
        //the variable common to all oxygen threads
        oxygenCount++;
        if(hydrogenCount>=2 && oxygenCount>=1){
            //can create an H2O molecule
            //signal any hydrogen threads which were waiting
            //to create a water molecule
            signal(waitingHydrogen);
            signal(waitingHydrogen);
            hydrogenCount-=2;
            //since a molecule would be created, two hydrogen threads
            //would be used up

            //signal any oxygen threads which were waiting
            //to create a water molecule
            signal(waitingOxygen);
            oxygenCount-=1;
            //since a molecule would be created, one oxygen thread
            //would be used up
        }else{
            //can't create a water molecule, allow other threads to execute
            signal(mutex);
        }
        wait(waitingOxygen);
        //wait on the waitingOxygen semaphore to create a water molecule
        //upto when two hydrogen threads are available
        createWater();
        //create bond when all the necessary threads to create
        //the molecule are available
        signal(mutex);
        //while an H2O molecule is being created by bonding,
        //the mutex is locked.
        //any of the threads creating the molecule can relase the mutex
        //once the molecule is created
        //we just assign this responsibility to the
        //oxygen thread which created the molecule
    }
}

void* hydrogenFunction(){
    //function called by hydrogen threads
    while(1){
        wait(mutex);
        //gain access to mutex so as to modify 
        //the variable common to all hydrogen threads
        hydrogenCount++;
        if(hydrogenCount>=2 && oxygenCount>=1){
            //can create an H2O molecule
            //signal any hydrogen threads which were waiting
            //to create a water molecule
            signal(waitingHydrogen);
            signal(waitingHydrogen);
            hydrogenCount-=2;
            //since a molecule would be created, two hydrogen threads
            //would be used up

            //signal any oxygen threads which were waiting
            //to create a water molecule
            signal(waitingOxygen);
            oxygenCount-=1;
            //since a molecule would be created, one oxygen thread
            //would be used up
        }else{
            //can't create a water molecule, allow other threads to execute
            signal(mutex);
        }
        wait(waitingHydrogen);
        //wait on the waitingOxygen semaphore to create a water molecule
        //upto when one other hydrogen thread and an oxygen thread are available
        createWater();
        //create bond when all the necessary threads to create
        //the molecule are available
    }   
    return NULL;
}

int main(int argc,  char* argv[]){
    //create semaphores
    waitingHydrogen=createSemaphore(0);
    waitingOxygen=createSemaphore(0);
    mutex=createSemaphore(1);

    //create threads
    for(;;){
        pthread_t o, h1, h2;
        pthread_create(&o, 0, &oxygenFunction, NULL);   
        pthread_create(&h1, 0, &hydrogenFunction, NULL);
        pthread_create(&h2, 0, &hydrogenFunction, NULL);
    }
    //infinite loop which keeps creating water molecules 
    //until we terminate the program
}