#define _CRT_SECURE_NO_WARNINGS
#define HAVE_STRUCT_TIMESPEC


#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sched.h"
#include "pthread.h"
#include "semaphore.h"
#include "time_functions.h"

#define READER 1
#define WRITER 0


typedef struct {

	bool type;
	int id_number;
	int delay_ms;
	char filepathout[32];
} thread_info;	


void *reader(void *reader_info);
void *writer(void *writer_info);


int *db_time_seconds;
int *db_time_milliseconds;

int readers_inside = 0;
int writers_waiting = 0;
bool writer_inside = false;

FILE *Google_Drive;
pthread_mutex_t mutex;
pthread_cond_t admit_readers, admit_writer;


int main() {

/* --- Professor Foreman's code --- */

	FILE *ifp;
	char *fullfilepath_in = (char *)malloc(32);
	char *thisprojectfile_in = (char *)malloc(16);
	strcpy(thisprojectfile_in, "project4_in.txt");
	strcpy(fullfilepath_in, filebase);
	strcat(fullfilepath_in, thisprojectfile_in);
	printf("fullpath=[%s] \n", fullfilepath_in);
	ifp = fopen(fullfilepath_in, "r");
	if (ifp == NULL)
	{
		printf("cannot open file_in");
		exit(4);
	}

	char *fullfilepath_out = (char *)malloc(32);
	char *thisprojectfile_out = (char *)malloc(16);
	strcpy(thisprojectfile_out, "project4_out.txt");
	strcpy(fullfilepath_out, filebase);
	strcat(fullfilepath_out, thisprojectfile_out);
	printf("fullpath=[%s] \n", fullfilepath_out);
	Google_Drive = fopen(fullfilepath_out, "w");
	if (Google_Drive == NULL)
	{
		printf("cannot open file_out");
		exit(4);
	}

/* --- end of P. Foreman's code --- */

	db_time_seconds = malloc(sizeof(int));
	db_time_milliseconds = malloc(sizeof(int));
	*db_time_seconds = 0;
	*db_time_milliseconds = 0;

	int r, w, R, W;				// #readers, #writers, reader delay, writer delay
	fscanf(ifp, "%d ", &r);
	fscanf(ifp, "%d ", &w);
	fscanf(ifp, "%d ", &R);
	fscanf(ifp, "%d ", &W);
	printf("%d %d %d %d\n", r, w, R, W);

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&admit_readers, NULL);
	pthread_cond_init(&admit_writer, NULL);
	
	printf("Initialized mutex and condition variables\n");
	
	pthread_t *reader_threads = malloc(sizeof(pthread_t) * r);
	pthread_t *writer_threads = malloc(sizeof(pthread_t) * w);
	thread_info reader_info[10];
	thread_info writer_info[10];
	
	int i;
	for (i = 0; i < r; i++) {
		reader_info[i].type = READER;
		reader_info[i].id_number = i + 1;
		reader_info[i].delay_ms = R;
		strcpy(reader_info[i].filepathout, fullfilepath_out);

		writer_info[i].type = WRITER;
		writer_info[i].id_number = i + 1;
		writer_info[i].delay_ms = W;
		strcpy(writer_info[i].filepathout, fullfilepath_out);
	}
	thread_info *temp_reader = malloc(sizeof(thread_info));
	thread_info *temp_writer = malloc(sizeof(thread_info));
	for (i = 0; i < r; i++) {
		temp_reader = &reader_info[i];
		temp_writer = &writer_info[i];
		pthread_create(&reader_threads[i], NULL, reader, (void *)temp_reader);
		pthread_create(&writer_threads[i], NULL, writer, (void *)temp_writer);
		pthread_join(reader_threads[i], NULL);
		pthread_join(writer_threads[i], NULL);
	}
	fclose(ifp);
	fclose(Google_Drive);
	system("PAUSE");
    return 0;
}


void *reader(void *reader_info) {
	
	thread_info *info;
	info = (thread_info *)reader_info;
	int i;
	int j;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < info->delay_ms; j++) millisleep(1);			// delay
		if (writers_waiting != 0 || writer_inside) pthread_cond_wait(&admit_readers, &mutex);		// if there is a writer waiting, wait for the writer queue to be pronounced empty
			

		readers_inside++;
		fprintf(Google_Drive, ">>> DB value read =: 0%d:%d by reader number: %d\n", *db_time_seconds, *db_time_milliseconds, info->id_number);
		printf(">>> DB value read =: 0%d:%d by reader number: %d\n", *db_time_seconds, *db_time_milliseconds, info->id_number);
		printf("readers inside = %d\nwriters waiting = %d\n", readers_inside, writers_waiting);
		readers_inside--;
		

		if (readers_inside == 0) pthread_cond_signal(&admit_writer);		// if last one out, signal any writer which may or may not be waiting
	}
}


void *writer(void *writer_info) {

	thread_info *info;
	info = malloc(sizeof(thread_info));
	info = (thread_info *)writer_info;
	int i;
	int j;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < info->delay_ms; j++) millisleep(1);			// delay
		if (readers_inside != 0 || writers_waiting != 0 || writer_inside) {					// if the db is NOT EMPTY
			writers_waiting++;									// announce that there is a writer waiting
			pthread_cond_wait(&admit_writer, &mutex);		// wait
			writers_waiting--;
		}
		writer_inside = true;										// announce that there is a writer inside
		get_wall_time_ints(db_time_seconds, db_time_milliseconds);
		fprintf(Google_Drive, ">>> DB value read =: 0%d:%d by reader number: %d\n", *db_time_seconds, *db_time_milliseconds, info->id_number);
		printf("*** DB value set to: 0%d:%d by writer number: %d\n", *db_time_seconds, *db_time_milliseconds, info->id_number);

printf("readers inside = %d\nwriters waiting = %d\n", readers_inside, writers_waiting);
if(writers_waiting != 0) printf("writer waiting\n");

		if (writers_waiting != 0) pthread_cond_signal(&admit_writer);		// if there is another writer waiting, signal it to enter
		else {
			writer_inside = false;
			pthread_cond_broadcast(&admit_readers);					// signal readers to enter
		}
	}
}
