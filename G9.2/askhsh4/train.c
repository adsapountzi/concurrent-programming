//  * Authors : Athanasia Despoina Sapountzi 2624
//  *           Vasilis Theiou               2685
//  * 16/11/19
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "mysem.h"

volatile int waiting = 0;
volatile int boarding = 0;
volatile int /*count = 0, */train_capacity, num_passengers, ride_duration;

// ==> BINARY SEMAPHORES <==
volatile int mtx;
volatile int wait_line;
volatile int exit_train;
volatile int train_full;
volatile int train_empty;
volatile int train_start;
volatile int join;


void passenger_enter(int id) {
    mysem_down(mtx);
    if(waiting == train_capacity && boarding == 0) {
//         printf("waiting %d\n", waiting); 
        mysem_up(train_full, __LINE__);
    }
    waiting++;
    printf("passenger waits %d ID %d!\n", waiting, id); 
    mysem_up(mtx, __LINE__);
    mysem_down(wait_line);
    printf("passenger %d boards on train.\n", id);
    mysem_down(mtx);
    if( boarding < train_capacity) { // each passenger that is on the train wakes up the passenger behing him in waiting line until train reaches its full capacity
        waiting--;
        boarding++;
//         count++;
        mysem_up(mtx, __LINE__);
        mysem_up(wait_line, __LINE__);
    }
    else if( boarding == train_capacity) { //Train is full !! notify train to start!!
        printf("Train is full.\n");
        mysem_up(mtx, __LINE__);
        mysem_up(train_start, __LINE__);
    }
}

void passenger_exit(int id) {  
    mysem_down(exit_train); //wait until train finishes its riiiiide
    mysem_down(mtx);
    printf("passenger exited %d\n", id);
    if(boarding == 0) {  //notify train that all passengers got off
        mysem_up(mtx, __LINE__);
        mysem_up(train_empty, __LINE__);
    }
    else if(boarding > 0) {//wake next passenger to exit train
        boarding--;
        mysem_up(mtx, __LINE__);
        mysem_up(exit_train, __LINE__);
    }
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
//        printf("WAITING %d\n", waiting);
        mysem_down(mtx);
        if(waiting < train_capacity){
            mysem_up(mtx, __LINE__);
            printf("Train waits until passengers reach train capacity.\n");
            mysem_down(train_full); //train waits till passengers are enough
        }
        else{
            mysem_up(mtx, __LINE__);
        }
        //passengers get on board!
        mysem_down(mtx);
        waiting--;
        boarding++;
//         count++;
        mysem_up(mtx, __LINE__);
        mysem_up(wait_line, __LINE__); //wake the first passenger in line to get on board!
        mysem_down(train_start); //train waits until passenger get on board!
        printf("Train starts!\n");
        sleep(ride_duration); //JUST RIIIIIDE
        printf("Train finished!\n");
        mysem_down(mtx);
        boarding--;
        mysem_up(mtx, __LINE__);
        mysem_up(exit_train, __LINE__); //wakes up the first passenger in order to exit the train.
        mysem_down(train_empty); //train waits until all passengers get off the train!
       printf("train is EMPTY\n");
//         if(count == num_passengers){
//             mysem_up(join, __LINE__);
//             return (NULL);
//         }
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

    // 	create and initialize binary semaphores.
    mtx = mysem_create(1);
	wait_line = mysem_create(0);
    exit_train= mysem_create(0);
    train_full = mysem_create(0);
    train_empty = mysem_create(0);
    train_start = mysem_create(0);
	join = mysem_create(0);
	
	pthread_create(&thread_train, NULL, train, NULL );
    
	for(i = 0; i < num_passengers; i++) {
        arr[i] = i;
		pthread_create(&passengers[i], NULL, passenger, &arr[i]);
	}

    mysem_down(join); //waits until all threads terminate
	
	mysem_destroy(mtx);
	mysem_destroy(wait_line);
    mysem_destroy(exit_train);
    mysem_destroy(train_full);
    mysem_destroy(train_empty);
    mysem_destroy(train_start);
    mysem_destroy(join);
    printf("CLOSE\n");
	return (0);
}
 
