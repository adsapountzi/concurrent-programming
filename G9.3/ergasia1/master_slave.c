/*This program creates a master thread and worker threads
 * master assigns jobs to them.
 * Authors : Athanasia Despoina Sapountzi 2624
 *           Vasilis Theiou               2685
 * 14/12/19
 * */
#include <stdio.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <pthread.h>



volatile int num_workers, size = 0,waiting = 0, jobs_done = 0;
 int *input;

pthread_cond_t waitq = PTHREAD_COND_INITIALIZER; 
pthread_cond_t work = PTHREAD_COND_INITIALIZER; 
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t terminate = PTHREAD_COND_INITIALIZER;
pthread_cond_t main_wait = PTHREAD_COND_INITIALIZER;
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
//     printf("maou %d\n", id);
//            pthread_mutex_lock(&mtx); 
    
	while (1) {
            printf("get %d\n", id);
            pthread_mutex_lock(&mtx); 
            waiting++;
            if(waiting == 1) {
                pthread_cond_signal(&main_wait);
            }
                
//         printf("Worker %d is available.\n", id);
        if(waiting == num_workers && jobs_done == size) { //Notify main that workers finished
            pthread_cond_signal(&work);
        }

        printf("get in line %d\n", id);
        pthread_cond_wait(&waitq, &mtx);
        if(jobs_done == size) {
            waiting--;
            
            if( waiting == 0){
                printf("Worker %d terminates!\n", id);
                pthread_mutex_unlock(&mtx);
                pthread_cond_signal(&terminate);
                return (NULL);
           }
           else{
                printf("Worker %d terminates!\n", id);
                pthread_mutex_unlock(&mtx);
                return (NULL);               
            }
        }
        else{
            temp = jobs_done;
            jobs_done++;
            printf("Worker %d got a job.\n", id);
			result = primetest(input[temp]);		
			if(result == 0) {
				printf("Number = %d is not prime worker %d\n", input[temp], id);
			}
			else if(result == 1) {
				printf("Number = %d is prime worker %d\n", input[temp], id);
			}          
		}
		pthread_mutex_unlock(&mtx);
    }
}

int main(int argc, char *argv[]) {
	int i, assigns = 0;
	int  result, tmp = 10; 
	int check;
    // Create and initialize binary semaphores. 
	
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
        pthread_mutex_lock(&mtx);
  	   /* wait for a worker to become available*/
        if(waiting == 0) {
            pthread_cond_wait(&main_wait, &mtx);
        }
        
        printf("WAITING %d\n", waiting);
        waiting--;
        assigns++;
        // wakeup worker to get job
          pthread_mutex_unlock(&mtx);
        pthread_cond_signal(&waitq);
      
	}
    
    pthread_mutex_lock(&mtx);

    if(waiting < num_workers) {
        printf("main waits till workers finish!\n");
        pthread_cond_wait(&work, &mtx);
    }
    
    for(i = 0; i< num_workers; i++) {
       // wake up workers!
        pthread_cond_signal(&waitq);
    }
//                 pthread_mutex_unlock(&mtx);

    printf("Main waits workers to terminate %d\n", waiting);
//     mysem_down(terminate); // Main waits until all workers terminate
    pthread_cond_wait(&terminate, &mtx);
    printf("main terminates!\n");
    pthread_mutex_unlock(&mtx);

	free(input);
	return (0);
}
