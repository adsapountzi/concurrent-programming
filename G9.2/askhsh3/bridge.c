//  * Authors : Athanasia Despoina Sapountzi 2624
//  *           Vasilis Theiou               2685
//  * 16/11/19
#include "mysem.h"
#include <unistd.h>
#include <pthread.h>

#define BLUE 0
#define RED 1

struct car_args {
    int color;
    int id;
};

volatile int b_waiting = 0, r_waiting = 0;
volatile int b_bridge = 0, r_bridge = 0;
volatile int bridge_size, cars_lim;
volatile int b_lim = 0, r_lim = 0;
volatile int num_cars = 0, input; 

//  ==> BINARY SEMAPHORES <==
volatile int mtx;
volatile int b_q, r_q;
volatile int join;

int rand_color( int num) {
    return rand() % (num + 1);
}

void enter_bridge(int car_color, int id){
    mysem_down(mtx);
    if(car_color == BLUE) {
        if(r_bridge > 0 ||  b_bridge == bridge_size || b_lim >=  cars_lim || b_waiting > 0) { //diff color on bridge OR bridge is full loaded
            b_waiting++;
//  			printf("WAIT CAR BLUE %d\n", id);
            mysem_up(mtx, __LINE__); 
            mysem_down(b_q); //blocked in queue
            
// 			printf("EXIT WAIT CAR BLUE %d\n", id);
//  			printf("WAITINGS = %d, ON BRIDGE %d\n", b_waiting, b_bridge);			
 			mysem_down(mtx);
            if( b_waiting > 0 &&  b_bridge < bridge_size && b_lim < cars_lim  ) {
                b_waiting--;
               
                b_bridge++;
        
                b_lim++;
				mysem_up(mtx, __LINE__);
                mysem_up(b_q, __LINE__);
            }
            else {
			mysem_up(mtx, __LINE__);
			}
// 			  			printf(" ON BRIDGE NUM %d COLOR BLUE\n", id);

        }
        else { 
             
            b_bridge++;
        
            b_lim++;
//              printf(" ON BRIDGE NUM %d COLOR BLUE\n", id);
			mysem_up(mtx, __LINE__); 
        }
    }
    if(car_color == RED) {
        if(b_bridge > 0 ||  r_bridge == bridge_size ||  r_lim >= cars_lim || r_waiting > 0) { //diff color on bridge OR bridge is full loaded
            r_waiting++;
//              printf("WAIT CAR RED %d\n", id);
            mysem_up(mtx, __LINE__); 
            mysem_down(r_q); //blocked in queue
// 			printf("EXIT WAIT CAR RED %d\n", id);
// 			printf("WAITINS = %d, ON BRIDGE %d\n", r_waiting, r_bridge);
 			mysem_down(mtx);
            if( r_waiting > 0 &&  r_bridge < bridge_size && r_lim < cars_lim ) {
                r_waiting--;
                r_bridge++;
                r_lim++;
                mysem_up(mtx, __LINE__);
                mysem_up(r_q, __LINE__);
            }
            else {
			mysem_up(mtx, __LINE__);
			}
        }
        else { 
             r_bridge++;
             r_lim++;
//               printf(" ON BRIDGE NUM %d COLOR RED\n", id);
			 mysem_up(mtx, __LINE__); 
        }
//         printf(" ON BRIDGE NUM %d COLOR RED\n", id);
    }
}

void exit_bridge(int car_color, int id) {
    mysem_down(mtx);
    if(car_color == BLUE) {

		b_bridge--;
        num_cars++;
//         printf(" BLUE car %d exits bridge, rem = %d!\n", id, b_bridge);
        if( b_bridge == 0 && r_waiting > 0) {
            if(b_lim >= cars_lim && b_bridge == 0) {
                printf("reached blue lim !\n");    
		   }
			b_lim = 0;
            r_waiting--;
            r_bridge++;
			r_lim++;
            mysem_up(mtx, __LINE__);
			mysem_up(r_q, __LINE__);
        }
//         else if( (b_waiting > 0 /*&& b_bridge < bridge_size*/ && b_lim < cars_lim) || (b_waiting > 0 /*&& b_bridge < bridge_size*/ && r_waiting == 0)) {
        else if( b_waiting > 0  && (b_lim < cars_lim || r_waiting == 0)) {
        
            b_waiting--;
            b_bridge++;
                 
            b_lim++;
			mysem_up(mtx, __LINE__);
            mysem_up(b_q, __LINE__);
        }
        else {
           if( num_cars == input) {
                mysem_up(mtx, __LINE__);
                 mysem_up(join, __LINE__);
             }
 			else {
                mysem_up(mtx, __LINE__);
            }
        }
        
    }
    if(car_color == RED) {
//         printf(" RED car %d exits bridge, rem = %d!\n", id, r_bridge);

        r_bridge--;
        num_cars++;
//  		if(r_bridge == bridge_size) {
// 			printf("BRIDGE FULL OF REDS\n");
// 		}
        
        if(r_bridge == 0  && b_waiting > 0) { //??///
			
            if(r_lim >= cars_lim) {
 				printf("reached red lim !\n");    
 			}
			r_lim = 0;
            b_waiting--;
            b_bridge++;
            b_lim++;
            mysem_up(mtx, __LINE__);
			mysem_up(b_q, __LINE__);
        }
//         else if( (r_waiting > 0 && r_bridge < bridge_size && r_lim < cars_lim) || (r_waiting > 0 && r_bridge < bridge_size && b_waiting == 0)) {
        else if( r_waiting > 0  && (r_lim < cars_lim || b_waiting == 0)) {
            r_waiting--;
            r_bridge++;
            r_lim++;
			mysem_up(mtx, __LINE__);
            mysem_up(r_q, __LINE__);
        }
        else {
             if( num_cars == input) {
                mysem_up(mtx, __LINE__);
                 mysem_up(join, __LINE__);
             }
 			else{
                mysem_up(mtx, __LINE__);
            }
 		}
    }    
}
    

void *car(void *thread_arg){
    struct car_args parameters;
    parameters = *(struct car_args *)thread_arg;
    enter_bridge(parameters.color, parameters.id);
    if(parameters.color == BLUE) {
        printf(" ON BRIDGE number %d color BLUE\n", parameters.id);
    }
    else{ 
        printf(" ON BRIDGE number %d color RED\n", parameters.id);
    }
        
    sleep(1);
    if(parameters.color == BLUE) {
                printf(" BLUE car %d exits bridge!\n", parameters.id);
    }
    else{
        printf(" RED car %d exits bridge!\n", parameters.id);
    }
    exit_bridge(parameters.color, parameters.id);
    return (NULL);
}

int main(int argc, char *argv[]) {

    if(argc != 3) {
        printf("Wrong number of arguements! \n");
        exit(1);
    }

    input = atoi(argv[2]);
    if(input == 0) {
        printf("0 cars enter bridge!\n"); 
        exit(-1);
    }

    int i, value;
    struct car_args vehicle[input];
    pthread_t thread[input];

    b_q = mysem_create(0);
    r_q = mysem_create(0);
    mtx = mysem_create(1);
    join = mysem_create(0);

    bridge_size = atoi(argv[1]);
    cars_lim = 2*bridge_size; //limit until colors switch

    for(i = 0; i < input; i++) {
        vehicle[i].color = rand_color(1);
        vehicle[i].id = i;
        value = pthread_create(&thread[i], NULL,  car , &vehicle[i] );
        if(value != 0) {
            printf("Error in pthread_create!\n");
            exit(1);
        }
    }

    if(b_q > 0 || r_q > 0){
        mysem_down(join);
    }
    
    mysem_destroy(mtx);
    mysem_destroy(b_q);
    mysem_destroy(r_q);
    mysem_destroy(join);

    return(0);
}
