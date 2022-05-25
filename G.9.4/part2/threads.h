#ifndef _LIST_H_
#define _LIST_H_
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <utime.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#define RUNNING 58
#define WAIT 0
#define MAIN -1
 #define SIZE 4
#define HEAD -1

typedef struct coroutine{

  ucontext_t context;

  struct coroutine *next;

  struct coroutine *prev;

  int key;

  int status;

}co_t;

typedef struct arguments{
	int key;
}args;		

typedef struct semaphores{
	int val;
}sem_t;



extern co_t *head;

extern co_t *schedule;

extern volatile int returned;
//list

void init_list();

int add_node (int key);

int remove_node(int key);

struct coroutine *find_node(int key);

struct coroutine *find_next_node(struct coroutine *current);

struct coroutine *find_running_node(int status);

int mysem_create(key_t key, int sem_number);

int mysem_init(int semid, int value);

int mysem_up(int sem_id);

int mysem_down(int sem_id);

int mysem_destroy(int sem_id);

int mycoroutines_init(co_t *co_main);

int mycoroutines_create(co_t *co,void (*body)(void),void *args);

int mycoroutines_swap(co_t *co ,co_t *uco);

int mycoroutines_destroy(co_t *co);

int mythreads_init();

int mythreads_create(co_t *co,void (*body)(void),void *args);

int mythreads_join(co_t *threads);

int mythreads_yield();

int mythreads_destroy(co_t *threads);

int mythreads_exit(int key);


int mythreads_sem_init(sem_t *s,int val);

int mythreads_sem_up(sem_t *s);

int mythreads_sem_down(sem_t *s);

int mythreads_sem_destroy(sem_t *s);


int main(int argc,char *argv[]);

#endif
