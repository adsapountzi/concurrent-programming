//  * Authors : Athanasia Despoina Sapountzi 2624
//  *           Vasilis Theiou               2685
//  * 14/12/19
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
// #include "mysem.h"

volatile int waiting = 0;
volatile int boarding = 0, empty = 0, full = 0, start = 0;
volatile int /*count = 0, */train_capacity, num_passengers, ride_duration;

pthread_cond_t wait_line= PTHREAD_COND_INITIALIZER; 
pthread_cond_t exit_train= PTHREAD_COND_INITIALIZER;
pthread_cond_t train_full= PTHREAD_COND_INITIALIZER; 
pthread_cond_t train_empty= PTHREAD_COND_INITIALIZER;
pthread_cond_t train_start= PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; 

void passenger_enter(int id) {
    pthread_mutex_lock(&mtx); 
    waiting++;

    if(waiting == train_capacity && full) {
//         printf("waiting %d\n", waiting);
        full = 0;
        pthread_cond_signal(&train_full);            
    }
    printf("passenger waits %d ID %d!\n", waiting, id); 
    pthread_cond_wait(&wait_line, &mtx);
    printf("passenger %d boards on train.\n", id);
//     mysem_down(mtx);
    waiting--;
    boarding++;
    if( boarding < train_capacity) { // each passenger that is on the train wakes up the passenger behing him in waiting line until train reaches its full capacity
       
//         count++;
//         mysem_up(mtx, __LINE__);
        pthread_cond_signal(&wait_line);            
    
//         mysem_up(wait_line, __LINE__);
    }
    else if( boarding == train_capacity) { //Train is full !! notify train to start!!
        printf("Train is full.\n");
//         mysem_up(mtx, __LINE__);
        if(start){
        start = 0;
        pthread_cond_signal(&train_start);            
    }
    }
    pthread_mutex_unlock(&mtx); 
}

void passenger_exit(int id) {  
    pthread_mutex_lock(&mtx);
    pthread_cond_wait(&exit_train, &mtx);
 //wait until train finishes its riiiiide
//     mysem_down(mtx);
    printf("passenger exited %d\n", id);
    if(boarding == 0 && empty) {  //notify train that all passengers got off
//         mysem_up(mtx, __LINE__);
        empty = 0;
        pthread_cond_signal(&train_empty);            
    }
    else if(boarding > 0) {//wake next passenger to exit train
        boarding--;
//         mysem_up(mtx, __LINE__);
        pthread_cond_signal(&exit_train);            
    }
    pthread_mutex_unlock(&mtx); 
}

void *passenger(void *pass_args) {
    int id;
    id = *(int *)pass_args;
    passenger_enter(id);
// on train
    passenger_exit(id);
    return(NULL);
}

void *train() {
        
    while(1) {        
       printf("New ride.\n"/*, boarding*/);
       pthread_mutex_lock(&mtx);
//        printf("WAITING %d\n", waiting);
        if(waiting < train_capacity){
//             mysem_up(mtx, __LINE__);
            full = 1;
            printf("Train waits until passengers reach train capacity.\n");
            pthread_cond_wait(&train_full, &mtx);
 //train waits till passengers are enough
        }
        //passengers get on board!
//         waiting--;
//         boarding++;
        pthread_cond_signal(&wait_line);            //wake the first passenger in line to get on board!
        if(boarding < train_capacity) {
            start =1;
            pthread_cond_wait(&train_start, &mtx);
        }
//         mysem_down(train_start); //train waits until passenger get on board!
        printf("Train starts!\n");
        sleep(ride_duration); //JUST RIIIIIDE
        printf("Train finished!\n");

        boarding--;
        pthread_cond_signal(&exit_train);            

//         mysem_up(exit_train, __LINE__); //wakes up the first passenger in order to exit the train.
       if(boarding > 0) {
           empty = 1;
        pthread_cond_wait(&train_empty, &mtx);
       }
//train waits until all passengers get off the train!
       printf("train is EMPTY\n");
       pthread_mutex_unlock(&mtx);  
    }
}

int main(int argc, char *argv[]) {
    
    if (argc != 4 ) {
		printf("Wrong number of arguments! \n");
		exit (1);
    }
    train_capacity = atoi(argv[1]);
    num_passengers = atoi(argv[2]);
    ride_duration = atoi(argv[3]);
    
    int i;
    int arr[num_passengers];
	pthread_t thread_train;
	pthread_t passengers[num_passengers];

    // 	create and initialize binary 
	
	pthread_create(&thread_train, NULL, train, NULL );
    
	for(i = 0; i < num_passengers; i++) {
        arr[i] = i;
		pthread_create(&passengers[i], NULL, passenger, &arr[i]);
	}
    for(i = 0; i < num_passengers; i++) {
        pthread_join(passengers[i], NULL);
    } 
    pthread_join(thread_train, NULL);
    printf("CLOSE\n");
	return (0);
}
 
