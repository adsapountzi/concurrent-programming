#include "coroutines.h"
#include<stdio.h>
#include<malloc.h>
#include<memory.h>
#include<stdlib.h>
#include<pthread.h>
#include<error.h>
#include<unistd.h>
#include <limits.h>

#define SIZE 64


typedef struct sbuff_{
	char * buff;
 	int readingPointer;
 	int writingPointer;
 	int size;
 	int count;
} sbuff_t;



typedef struct arguments{
	FILE *fp;
	long size;
} args;//data structure to be passed as thread arguments. fp is each file's
//filedescriptor. size is the total amount of bytes the reading file has.
sbuff_t * sb = NULL;
int flag;
//this function initialises a pipe of size size, which is a variable that we
//pass as argument from main. it creates the circular buffer.
void pipe_init(int size){
 	sb = (sbuff_t*)malloc(sizeof(sbuff_t));
  	memset(sb, 0, sizeof(sbuff_t));
   	sb->size = size;
 	sb->buff = (char*)malloc(sizeof(char)*size);
	flag = 0;

}

void pipe_close() {
	flag = 1;
}
//this function writes a byte(a character) in our pipe. if the pipe has no empty
// spots it just busy waits untill an empty spot is found.
void pipe_write(char elem){

	sb->buff[sb->writingPointer] = elem;
	sb->writingPointer = (sb->writingPointer+1) % sb->size;
	sb->count ++;
}

//this function reads a byte(a character) from our pipe and then frees its position.
// if the pipe has no bytes it just busy waits untill a byte is writen.
int pipe_read(char *c){

	if (sb->count == 0) {
		return (0);
	}
	*c = sb->buff[sb->readingPointer];
	sb->readingPointer = (sb->readingPointer + 1 ) % sb->size;
	sb->count--;
	return (1);

}

//here we free the buffer.
void pipe_destroy(){
	free(sb->buff);
	free(sb);
}

//static ucontext_t uctx_main, uctx_func1, uctx_func2;
co_t main_c,produc_c,consum_c;
volatile int finished = 0;

static void producer(void *input){
	char c;
	args *arg;
	int ret;//return value of the fread command
	arg = (args *) input;
	int i = 0;

	printf("producer\n");
	//we use a for, since we have the file size, in order to read the whole file
	//byte-byte
	for (i = 0; i < arg->size; i++) {

		ret = fread(&c,1,1,arg->fp);
		if ( ret == 0) {//checking for errors
			perror("ERROR:");
			exit(EXIT_FAILURE);
		}
		pipe_write(c);//we call the function that writes in our pipe
		if ((i + 1) % SIZE == 0) {
			printf("%d\n", ((i+1)/SIZE));
			consum_c.from= &produc_c;
			printf("Switching from producer to consumer\n");
			mycoroutines_switchto(&consum_c);
		}
	}
	if (i % SIZE) {
		printf("%d\n", ((i+1)/SIZE)+1);
		consum_c.from= &produc_c;
		printf("Switching from producer to consumer\n");
		mycoroutines_switchto(&consum_c);
	}
	printf("producer: terminating\n");
	return;
}


static void consumer(void *input){
	char c;
	args *arg;
	int ret;//return value of the fwrite command
	arg = (args *) input;
	int i = 0;

	printf("consumer \n");
	//we use a for, since we have the file size, in order to read the whole file
	//byte-byte

	while(1) {
		ret = pipe_read(&c);//we call the function that reads from our pipe
		if(!ret){//pipe read returns 1 if it reads a char and 0 if there is no other
			//char to be read. so we break;
			break;
		}
		ret = fwrite(&c,1,1,arg->fp);
		if ( ret == 0) {//checking for errors
			perror("ERROR:");
			exit(EXIT_FAILURE);
		}
		if ((i + 1) % SIZE == 0) {
			produc_c.from= &consum_c;
			printf("Switching from consumer to producer\n");
			mycoroutines_switchto(&produc_c);
		}
		i = i + 1;
	}
	printf("consumer: terminating\n");
	return;
}

int main(int argc, char *argv[]){
	FILE *writefp,*readfp;//our 2 file descriptors
	char *producer_stack, *consumer_stack;
	char command[LINE_MAX];
	long size;
	args prod_args,cons_args;//the arguments we give in our 2 coroutines

	if (argc != 3){//if we have more arguments than the expected ones we print
		//the error and we exit.
		printf("ERROR:wrong number of arguments \n");
		return 0;
	}
	pipe_init(SIZE);
//we open the reading file
	readfp=fopen(argv[1],"r+b");
	if ( readfp == NULL) {
		perror("ERROR:");
		exit(EXIT_FAILURE);
	}
//on the reading file we use fseek with the SEEK_END option in order to go to
//the end of the file. then we use ftell which returns the current value of
//the file position indicator,which means the number of the last byte, and thus
//the size of the file. in the end we reset the filedescriptor to the start of
//our reading file.
	fseek(readfp,0,SEEK_END);
	size = ftell(readfp);
	fseek(readfp,0,SEEK_SET);
//we open the writing file
	writefp=fopen(argv[2],"w+b");
	if ( writefp == NULL ) {
		perror("ERROR:");
		exit(EXIT_FAILURE);
	}
//initialising reading arguments
	prod_args.size = size;
	prod_args.fp = readfp;
//initialising writing arguments
	cons_args.size = size;
	cons_args.fp = writefp;

	producer_stack = (char *) malloc(STACK_SIZE);
	consumer_stack = (char *) malloc(STACK_SIZE);
	produc_c.stack = producer_stack;
	produc_c.size_stack = STACK_SIZE;
	consum_c.stack = consumer_stack;
	consum_c.size_stack = STACK_SIZE;

	mycoroutines_init(&main_c);
	produc_c.link=&main_c;
	mycoroutines_create(&produc_c,&producer,&prod_args);
	consum_c.link=&produc_c;
	mycoroutines_create(&consum_c,&consumer,&cons_args);

	printf("main: swapcontext(&uctx_main, &uctx_func2)\n");
	produc_c.from= &main_c;
	mycoroutines_switchto(&produc_c);
	fclose(writefp);
	fclose(readfp);
	memset(command,0,sizeof(command));
	strcat(command,"diff ");
	strcat(command,argv[1]);
	strcat(command," ");
	strcat(command,argv[2]);
	strcat(command,"\n");
	printf("%s", command);
	system(command);
	printf("main: exiting\n");
	//after everything is finished we wrap up closing the file descriptors
	mycoroutines_destroy(&produc_c);
	mycoroutines_destroy(&consum_c);
	pipe_destroy();
	exit(EXIT_SUCCESS);
}
