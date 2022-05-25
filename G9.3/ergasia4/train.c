//  * Authors : Athanasia Despoina Sapountzi 2624
//  *           Vasilis Theiou               2685
//  * 14/12/19

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "ccrf.h"

CCR_DECLARE(R)
volatile int exiting = 0;
volatile int boarding = 0;
volatile int train_capacity, num_passengers, ride_duration;
// ==>  FLAGS <==
volatile int flag = 0;

void passenger_enter(int id) {
    CCR_EXEC(R, (boarding != train_capacity && flag == 0), {  
        boarding++;
        printf("passenger %d gets on board!\n", id);
    })
}
void passenger_exit(int id) {  
    
CCR_EXEC(R, (exiting != train_capacity && flag == 1), {  
        exiting++;
        printf("passenger exited %d \n", id);
    })
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
       printf("New ride.\n");
       CCR_EXEC(R,(boarding == train_capacity),{
			boarding = 0;
			flag = 1;
        })            //notify to board on dat train!
	   
	sleep(ride_duration); //JUST RIIIIIDE
		
		CCR_EXEC(R, (exiting == train_capacity), {
			exiting = 0;
			flag = 0;
        })
    }
}

int main(int argc, char *argv[]) {
    
    if (argc != 4 ) {
		printf("Wrong number of arguments! \n");
		exit (1);
    }

    CCR_INIT(R)

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
 
