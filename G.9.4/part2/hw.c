#include "threads.h"
#define MATRIX_SIZE 100
#define TIME 2

sem_t *mtx;
sem_t *read_num;
sem_t *write_num;
sem_t *wait_thread;
sem_t *end;

struct sigaction sigalarm={{0}};

int number;

int counter=0;

int counter_master=0;

int worker_ended=0;

int numbers[512]={0};

int count_temp;

struct itimerval timer;

int end_sched=0;

volatile sig_atomic_t stop=0;

int flag=0;

co_t *head;

co_t *schedule;

sigset_t mask;

args *create_args[512];

volatile int returned;

volatile int workers_number;

int primetest(int num);

void *worker_thread(void *input);

void *master_thread(void *input);

int mythreads_init();

int mythreads_create(co_t *co,void (*body)(void),void *args);

int mythreads_join(co_t *threads);

int mythreads_yield();

int mythreads_destroy(co_t *threads);

int mythreads_exit(int key);

int j, i;

static void sigalarm_handler(int sig){

	sigfillset(&mask);              
	
	sigprocmask(SIG_SETMASK,&mask,NULL);

	timer.it_interval.tv_usec=100;
	timer.it_value.tv_usec=100;
	struct coroutine *current;

    for(current=head->next;current!=head;current=current->next){ 
        if(current->status==RUNNING){
            current->status=WAIT;                                 
            break;
        }
    }

   	co_t *temporary=current;

    current=find_next_node(current);   

    if(current==head){
    	signal(SIGALRM,SIG_IGN);
		stop=1;
		return;
    	
    }  

    current->status=RUNNING;  
    setitimer(ITIMER_REAL,&timer,NULL);

    sigemptyset(&mask);               
	sigprocmask(SIG_SETMASK,&mask,NULL);

    mycoroutines_swap(temporary,current);                         
}


int main(int argc,char *argv[]){
	int i;
	int number_temp;

	timer.it_interval.tv_usec=100;
	timer.it_value.tv_usec=1000;

	workers_number= atoi(argv[1]);


	for(j=0;j<512;j++){
		create_args[j]=(args *)malloc(sizeof(args));
	}
	

	mtx=(sem_t *)malloc(sizeof(sem_t));

	read_num=(sem_t *)malloc(sizeof(sem_t));

	write_num=(sem_t *)malloc(sizeof(sem_t));

	wait_thread=(sem_t *)malloc(sizeof(sem_t));

	end=(sem_t *)malloc(sizeof(sem_t));


	mythreads_sem_init(mtx,1);
	mythreads_sem_init(read_num,0);
	mythreads_sem_init(write_num,0);
	mythreads_sem_init(wait_thread,0);
	mythreads_sem_init(end,0);


	count_temp=0;

	while(scanf("%d",&number_temp)!=EOF){

		numbers[count_temp]=number_temp;

		count_temp++;
	}

	counter_master=count_temp;
	mythreads_init();
	mythreads_create(find_node(1),(void(*)())master_thread,create_args[1]);

	for(i=0;i<workers_number;i++){

		mythreads_create(find_node(i+2),(void(*)())worker_thread,create_args[i+2]);

	}
	setitimer(ITIMER_REAL,&timer,NULL);
	mythreads_join(find_node(1));
	for(i=0;i<workers_number;i++){
		mythreads_join(find_node(i+2));
	}

	while(1){
		if(stop==1){
			break;
		}
	}

	return 0;
}

int mythreads_init(int key){     
														
	sigalarm.sa_handler=sigalarm_handler;

    
    if(sigaction(SIGALRM,&sigalarm,NULL)<0){

        perror("sigaction sigalarm");
    }

 
    init_list();


    add_node(0);

    mycoroutines_init(find_node(0));


    for(i=0;i<SIZE;i++){

        add_node(i+1);
    }

    return 0;
}


int mythreads_join(co_t *threads){

	while(1){
		if(threads->status==-1){
			printf("JOIN\n");
			break;
		}
	}
	return 0;
}

int mythreads_yield(){
	co_t *current;
	co_t *next;
	sigset_t set;



	sigemptyset(&set);

    sigaddset(&set,SIGALRM);

    sigprocmask(SIG_BLOCK,&set,NULL);

	current=find_running_node(RUNNING);

	current->status=WAIT;

	next=find_next_node(current);

	next->status=RUNNING;

	sigprocmask(SIG_UNBLOCK,&set,NULL);

	mycoroutines_swap(current,next);

	return 0;
}

int mythreads_destroy(co_t *threads){

	remove_node(threads->key);

	return 0;
}

int mythreads_exit(int key){
    co_t *current;

    for(current=head->next;current!=head;current=current->next){
        if(current->key==key){
            current->status=-1;
            return 1;
        }
    }
    return 0;
}
int mythreads_create(co_t *co,void (*body)(void),void *arg){
	mycoroutines_create(co,body,(void*)arg);
	return 0;
}

void *worker_thread(void *input){
	int num;
	int i=0;
	int k=0;

	while(1) {
		mythreads_sem_down(mtx);

		if(count_temp == 0){
			mythreads_sem_up(end);
			mythreads_sem_up(mtx);
			printf("worker %d terminate\n",((args*)input)->key);
			break;
		}

		mythreads_sem_up(mtx);

		mythreads_sem_down(write_num);

		mythreads_sem_down(mtx);

		if(count_temp==0){

			mythreads_sem_up(mtx);
			printf("worker %d terminate\n",((args*)input)->key);
			break;
		}
		mythreads_sem_up(mtx);
		mythreads_sem_down(mtx);	
		num=number;
		mythreads_sem_up(mtx);
		mythreads_sem_up(read_num);	


		i = 0;
		k = 0;

		for(i=2 ; i<=(num/2) ; ++i) {
			if(num%i == 0) {
				k=1;
		  		break;
			}
		}

		if(k == 0 && num != 1) {
			printf("worker %d: %d is Prime\n",((args*)input)->key,num);
		}
		else {
			printf("worker %d: %d is not Prime\n",((args*)input)->key,num); 
		}

		mythreads_sem_down(mtx);

		count_temp--;

		mythreads_sem_up(mtx);

		
	}

	if(mythreads_exit(((args*)input)->key)==1){
		returned++;
		return NULL;
	}

	return NULL;
	
}

void *master_thread(void *input){
	int i=0;

	while(1) {
		
		if(i==counter_master){	
			mythreads_sem_down(end);	
			for(i=0;i<workers_number;i++){			
				mythreads_sem_up(write_num);
			}
			break;
			
		}

		mythreads_sem_down(mtx);
		number=numbers[i];		
		mythreads_sem_up(mtx);
		mythreads_sem_up(write_num);	
	
		mythreads_sem_down(read_num);	
		i++;
	}

	if(mythreads_exit(((args*)input)->key)==1){
		returned++;
		return NULL;
	}

	return NULL;
}


