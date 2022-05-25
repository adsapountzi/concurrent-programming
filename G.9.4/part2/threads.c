#include "threads.h"


void init_list(){
    
    head = (struct coroutine *)malloc(sizeof(struct coroutine));

    head->key=90;

    head->status=-3;

    if (head == NULL) {

        printf("Memory allocation error in initialization\n");

        exit(1);
    }
    
    head->next = head;

    head->prev = head;
}   


int add_node (int key){
	struct coroutine *new_node;
	

	new_node = (struct coroutine *)malloc(sizeof(struct coroutine));

	if (new_node == NULL) {

		printf("Problem\n");

		return(1);
	}
	

    if(key==0){

        new_node->status=RUNNING;

    }
    else if(key==1){

       new_node->status=WAIT; 

    }

    else new_node->status=WAIT;

	new_node->key=key;

	new_node->next = head;

	new_node->prev = head->prev;
	
	new_node->next->prev = new_node;

	new_node->prev->next = new_node;

	return(0);
}

int remove_node (int key) {

    struct coroutine *current;
    
    head->key = key;
    
    for (current = head->next; current->key != key; current = current->next) {}
    
    if (current == head){
        return 0;
    }
        
    current->key=-13;

    head->key=-70;
    return(0);
}



struct coroutine *find_node(int key){

	struct coroutine *current;

	for(current=head->next;current!=head;current=current->next){

		if(current->key==key){

			return current;
		}
	}
 	return head;
}

struct coroutine *find_running_node(int status){

    struct coroutine *current;

    for(current=head->next;current!=head;current=current->next){

        if(current->status==status){

            return current;
        }
    }
    return head;
}





struct coroutine *find_next_node(struct coroutine *current){  
											
    while(1){
        current=current->next;

         if(returned==SIZE){ 
            return head;
        }

        if(current->status==0){
            return (current);

        }
    }
}


int mycoroutines_init(co_t *co_main){

    getcontext(&(co_main->context));

    co_main->context.uc_stack.ss_sp = malloc(64000);

    co_main->context.uc_stack.ss_size = 64000;

    co_main->context.uc_link = NULL;

    makecontext(&co_main->context,(void*)main,0);

    return 0;
}


int mycoroutines_create(co_t *co,void (*body)(void),void *arg){
    getcontext(&(co->context));
     
    co->context.uc_stack.ss_sp = malloc(64000);

    co->context.uc_stack.ss_size = 64000;

    co_t* temp=find_node(0);

    co->context.uc_link=&temp->context;

    ((args*)arg)->key=co->key;

    makecontext(&(co->context),(void(*)())body, 1,(void*)arg);
    
    return 0;
}



int mycoroutines_swap(co_t *co,co_t *uco){

    swapcontext(&(co->context),&(uco->context));

    return 0;
}



int mycoroutines_destroy(co_t *co){

    remove_node(co->key);

    return 0;
}

int mythreads_sem_init(sem_t *s,int val){
    s->val=val;
    return 0;
}


int mythreads_sem_up(sem_t *s) {
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);

    s->val++;

    sigprocmask(SIG_UNBLOCK, &set, NULL);

    return 0;
}

int mythreads_sem_down(sem_t *s){
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set,SIGALRM);
    sigprocmask(SIG_BLOCK,&set,NULL);

    while(1){


        if(s->val>0){
            s->val--;
            break;
        }
        else{
            sigprocmask(SIG_UNBLOCK,&set,NULL);

            mythreads_yield();

            sigemptyset(&set);
            sigaddset(&set,SIGALRM);
            sigprocmask(SIG_BLOCK,&set,NULL);

        }
    }

    sigprocmask(SIG_UNBLOCK,&set,NULL);
    return 0;
}

int mythreads_sem_destroy(sem_t *s){
    free(s);
    return 0;
}
