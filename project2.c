#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "sched.h"
#include "pthread.h"
#include "semaphore.h"

#include "time_functions.h"


#define BUFFER_SIZE		10
#define LINE_LENGTH		1001

typedef struct buffer_slot {

	char line[1001];
	bool EOF_flag;
}buffer_slot;

bool end = false;

int p_index = 0;
int c_index = 0;

char p_tempLine[1001];
char c_tempLine[1001];
buffer_slot bounded_buffer[10];

FILE *r_fname;
FILE *w_fname;

void producer();
void consumer();

void produce(FILE *r_fname);
void append();

void take();
void consume(FILE *w_fname);

sem_t sem_e, sem_f, sem_m;


int main(int argc, char *argv[]) {

	printf("Begin\n");
	/* Initialize all EOF_flags to false */
	int i;
	for (i = 0; i < 10; i++) {

		printf("Flag %d cleared.\n",i);
		bounded_buffer[i].EOF_flag = false;
	}

	/* Check to make sure the user has entered correct number of arguments */
	if (argc == 3) printf("Reading from: %s\nWriting to: %s\n", argv[1], argv[2]);
	else {

		printf("Incorrect number of arguments,\nPlease enter a file name to read from, and a file name to write to.\n");
		exit(0);
	}

	/* Initialize three semaphores; empty=10, full=0, mutex=1; all shared by threads */
	sem_init(&sem_e, 0, BUFFER_SIZE);
	sem_init(&sem_f, 0, 0);
	sem_init(&sem_m, 0, 1);

	/* Open files for reading and writing */
	r_fname = fopen(argv[1], "r");
	if (r_fname == NULL) {

		printf("Problem opening read file, try again.\n");
		exit(0);
	}

	w_fname = fopen(argv[2], "w");
	if (w_fname == NULL) {

		printf("Problem opening write file, try again.\n");
		exit(0);
	}

	/* Declare and prepare two threads */
	pthread_t prod_th, cons_th;

	start_timing();
	if (pthread_create(&prod_th, NULL, (void *)&producer, NULL) != 0) {

		printf("Error creating producer thread.\n");
		exit(0);
	}
	printf("Producer thread created.\n");
	if (pthread_create(&cons_th, NULL, (void *)&consumer, NULL) != 0) {

		printf("Error creating consumer thread.\n");
		exit(0);
	}
	printf("Consumer thread created.\n");
	
	pthread_join(prod_th, NULL);
	pthread_join(cons_th, NULL);
	stop_timing();

	fclose(r_fname);
	fclose(w_fname);

	printf("CPU time: %10.3f seconds\n", get_CPU_time_diff());
	printf("Wall Clock time: %10.3f seconds\n", get_wall_clock_diff());
	return 0;
}

//
/* Producer thread */
//
void producer() {

	//char tempLine[LINE_LENGTH];
	while(1) {

		produce(r_fname);
		sem_wait(&sem_e);
		sem_wait(&sem_m);
	printf("PRODUCER DECREMENTED MUTEX\n");
		if (end) {

			if(p_index == 0) bounded_buffer[BUFFER_SIZE - 1].EOF_flag = true;
			else bounded_buffer[p_index - 1].EOF_flag = true;
			printf("Finished reading!!\n");
			sem_post(&sem_m);
			printf("PRODUCER INCREMENTED MUTEX\n");
			printf("p_index: %d\nc_index: %d\n",p_index, c_index);
			break;
		}
	printf("p_index: %d\nc_index: %d\n",p_index, c_index);
		append();
		sem_post(&sem_m);
		sem_post(&sem_f);
	printf("PRODUCER INCREMENTED MUTEX\n");
	}
}

//
/* Consumer thread */
//
void consumer() {

	//char tempLine[LINE_LENGTH];
	while (1) {

		sem_wait(&sem_f);
		sem_wait(&sem_m);
	printf("CONSUMER DECREMENTED MUTEX\n");
		if (bounded_buffer[c_index].EOF_flag == true) {

			take();
			sem_post(&sem_m);
			sem_post(&sem_e);
			consume(w_fname);
			printf("Consumed %d lines.\n",k);
			printf("p_index: %d\nc_index: %d\n",p_index, c_index);
			printf("Finished writing!!\n");
			break;
		}
	printf("p_index: %d\nc_index: %d\n",p_index, c_index);
		take();
		sem_post(&sem_m);
		sem_post(&sem_e);
	printf("CONSUMER INCREMENTED MUTEX\n");
		consume(w_fname);
	printf("Consumed %d lines.\n",k);
	k++;
	}
}

//
/* Producer functions */
//
//produce() copies a line out of the input file and stores it in the producer buffer.
//append() places the line from the producer buffer into the bounded buffer.
void produce(FILE *r_fname) {

	char temp[LINE_LENGTH];
	printf("produce()\n");
	if(fgets(temp, LINE_LENGTH, r_fname) == NULL){

		end = true;
		printf("FOUND EOF!!\n");
		memcpy(p_tempLine, temp, (LINE_LENGTH*sizeof(char)));
	}
	printf("Produced %d lines.\n",j);
	j++;
	memcpy(p_tempLine, temp, (LINE_LENGTH*sizeof(char)));
}

void append() {

	printf("append()\n");
	strcpy(bounded_buffer[p_index].line, p_tempLine);
	p_index = (p_index + 1) % BUFFER_SIZE;
}

//
/* Consumer functions */
//
//take() copies a line out of the bounded buffer and stores it in the consumer buffer.
//consume() places the line from the consumer buffer into the output file.
void take() {

	char temp[LINE_LENGTH];
	printf("take()\n");
	strcpy(temp, bounded_buffer[c_index].line);
	memcpy(c_tempLine, temp, (LINE_LENGTH*sizeof(char)));
	c_index = (c_index + 1) % BUFFER_SIZE;
}

void consume(FILE *w_fname) {

	printf("consume()\n");
	fputs(c_tempLine, w_fname);
}
