#include<stdio.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0);

#define STACK_SIZE 16384

typedef struct co_data{
	ucontext_t uctx;
	char *stack;
	int size_stack;
	struct co_data *from;
	struct co_data *link;
} co_t;

int mycoroutines_init(co_t *main);

int mycoroutines_create(co_t *co, void (body)(void *),void *arg);

int mycoroutines_switchto(co_t *co);

int mycoroutines_destroy(co_t *co);



int mycoroutines_init(co_t *main){
	if (getcontext( &(main->uctx) ) == -1)
        handle_error("coroutine init:");
	//init co struct for main
	return 0;
}

int mycoroutines_create(co_t *co, void (body)(void *),void *arg){

    if (getcontext( &(co->uctx) ) == -1)
        handle_error("coroutine create:");
    co->uctx.uc_stack.ss_sp = co->stack;
    co->uctx.uc_stack.ss_size = co->size_stack;
    co->uctx.uc_link = &(co->link->uctx);
    makecontext(&(co->uctx) , (void (*)(void)) body, 1, arg);
	return 0;
}

int mycoroutines_switchto(co_t *co){
	if (swapcontext(&(co->from->uctx), &(co->uctx)) == -1)
		handle_error("coroutine switch(swap)");
	return 0;
}

int mycoroutines_destroy(co_t *co){
	free (co->stack);
	return 0;
} 
