#include <stdio.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

int mysem_create(int init) {
	int semid;
	int check;
	
	semid = semget(IPC_PRIVATE, 1, S_IRWXU);
	if(semid == -1) {
		printf("\nError in semget\n");
		exit (-1);
	}
	check = semctl(semid, 0, SETVAL, init);
	if(check == -1) {
		printf("\nError in semctl\n");
		exit (-1);
	}
	
	return (semid);
}

void mysem_down(int semid) {
	struct sembuf op;
	int check;
	
	op.sem_num = 0;
	op.sem_op = -1;
	op.sem_flg = 0;
	
	check = semop(semid, &op, 1);
	if(check == -1) {
		printf("\nError in semop down\n");
		exit (-1);
	}
}

int mysem_up(int semid, int line) {
	struct sembuf op;
	int check;
	int value;
	
	value = semctl(semid, 0, GETVAL);
	if(value == -1) {
		printf("\nError in semctl up\n");
		exit (-1);
	}
	if(value) {
        printf("\nSem is already 1 %d \n", line);
		return (-1);
	}
	
	op.sem_num = 0;
	op.sem_op = 1;
	op.sem_flg = 0;
	
	check = semop(semid, &op, 1);
	if(check == -1) {
		printf("\nError in semop up\n");
		exit (-1);
	}
	return (0);
}

void mysem_destroy(int semid) {
	int check;
	
	check = semctl(semid, 0, IPC_RMID);
	if(check == -1) {
		printf("\nError in semctl destroy\n");
		exit (-1);
	}
}


