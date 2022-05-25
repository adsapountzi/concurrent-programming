/*This program creates a master thread and worker threads
 * master assigns jobs to them.
 * Authors : Athanasia Despoina Sapountzi 2624
 *           Vasilis Theiou               2685
 * 16/11/19
 * */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "mysem.h"

volatile int num_workers, size = 0,waiting = 0, jobs_done = 0;
 int *input;
 // ==> BINARY SEMAPHORES <==
volatile int mtx;
volatile int waitq;
volatile int terminate, work;

int primetest(int num) {
	int i;
	if (num <= 1) {
		return (0);
	}
	if (num % 2 == 0 && num > 2) {
		return (0);
	}
	for(i = 3; i < num / 2; i+= 2) {
		if (num % i == 0) {
			return 0;
		}
	}
	return 1;
}

void *worker(void *threadarg) {
	int result, temp;
	int id;
    id = *(int *)threadarg;
	
	while (1) {
        mysem_down(mtx);
        waiting++;
        
//         printf("Worker %d is available.\n", id);
        if(waiting == num_workers && jobs_done == size) { //Notify main that workers finished
            mysem_up(work, __LINE__);
        }
        mysem_up(mtx, __LINE__);
        mysem_down(waitq); //  get in line to work
        mysem_down(mtx);
        if(jobs_done == size) {
            waiting--;
            if( waiting == 0){
                printf("Worker %d terminates!\n", id);
                mysem_up(mtx, __LINE__);
                mysem_up(terminate, __LINE__); //All workers terminated! Notify main to terminate! 
                return (NULL);
           }
           else{
                printf("Worker %d terminates!\n", id);
                mysem_up(mtx, __LINE__);
                return (NULL);               
            }
        }
        else{
            temp = jobs_done;
            jobs_done++;
            printf("Worker %d got a job.\n", id);
			result = primetest(input[temp]);		
			 mysem_up(mtx, __LINE__);
			if(result == 0) {
				printf("Number = %d is not prime worker %d\n", input[temp], id);
			}
			else if(result == 1) {
				printf("Number = %d is prime worker %d\n", input[temp], id);
			}          
		}
    }
}

int main(int argc, char *argv[]) {
	int i, assigns = 0;
	int  result, tmp = 10; 
	int check;
    // Create and initialize binary semaphores. 
	work = mysem_create(0);
    mtx = mysem_create(1);
    waitq = mysem_create(0);
    terminate = mysem_create(0);
    if (argc != 2 ) {
		printf("Wrong number of arguments! \n");
		exit (1);
	}
	input = malloc(sizeof(int) * 10);
	
	while(1) {
		result =  scanf("%d", &input[size]);
		size++;
		if (result == EOF) {    //if input reaches end of file
			size--;
			break;  
		}
		if(size == tmp - 1) {   //Reallocate the array
			tmp = 2*tmp;
			input = (int *) realloc(input, sizeof(int)* tmp);
		}
	}
	
	input = (int *) realloc(input, sizeof(int)* size);
	
	num_workers = atoi(argv[1]);
    int array[num_workers];
	pthread_t thread[num_workers];
	
	for (i = 0; i < num_workers; i++) {
		array[i] = i;
		check = pthread_create(&thread[i], NULL,  worker , &array[i] );
		if(check != 0) {
			printf("Error in pthread_create!\n");
			exit(1);
		}
	}
// 	printf("JOBS DONE %d SIZE %d WORKERS %d\n\n",  jobs_done, size, num_workers);

    while (assigns < size) {
	   /* wait for a worker to become available*/
        mysem_down(mtx);
        if(waiting > 0) {
//             printf("WAITING %d\n", waiting);
            waiting--;
            assigns++;
            mysem_up(mtx, __LINE__);
            mysem_up(waitq, __LINE__);  // wakeup worker to get job
        }
        else{
            mysem_up(mtx, __LINE__);
        }
	}
            
    mysem_down(mtx);        
    if(waiting < num_workers) {
        mysem_up(mtx, __LINE__);
        printf("main waits till workers finish!\n");
        mysem_down(work);
    }
    else {
        mysem_up(mtx, __LINE__);
    }
        
   
    for(i = 0; i< num_workers; i++) {
        mysem_up(waitq, __LINE__); // wake up workers!	
    }

    printf("Main waits workers to terminate %d\n", waiting);
    mysem_down(terminate); // Main waits until all workers terminate
    printf("main terminates!\n");
    
    mysem_destroy(mtx);
    mysem_destroy(work);
    mysem_destroy(waitq);
    mysem_destroy(terminate);
	
	free(input);
	return (0);
}


 
