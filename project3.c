#define HAVE_STRUCT_TIMESPEC


#include <stdio.h>
#include <stdlib.h>
#include "sched.h"
#include "pthread.h"
#include "semaphore.h"
#include "time_functions.h"


typedef struct {

	int row;
	int column;
} coordinate;

typedef struct {

	coordinate c;
	int value;
} element;

typedef struct {

	int rows;
	int columns;
	element *matrix;
} matrix;

typedef struct {

	int a;
	int b;
	coordinate c;
} packet;

matrix *createMatrix();
void getInputDimensions(FILE *f);
void fillMatrices(FILE *f);
void *getProduct(void *p);


//	declaration of GLOBAL data
//	in:  matrix A, matrix B
//	out: matrix C
//	semaphore array

matrix *A;
matrix *B;
matrix *C;

sem_t *mutex_array;

int main(int argc, char *argv[]) {
	
	FILE* ifp;
// create a place where all the pieces will be concatenated
	char *fullfilepath_in = (char *)malloc(30);
// allow for different names per project
	char *thisprojectfile_in = (char *)malloc(13);
	strcpy(thisprojectfile_in, "project3_in.txt");
/* All students will have to use the SAME names & paths for thisprojectfile_in & thisprojectfile_out.
	"filebase" (defined in time_functions.h) is the path to your data, minus the file name.
	On Windows it is: c:\oscourse on Linux it is /root/oscourse
	The code in "time_functions.h will detect Windows/Linux for you */
	
	strcpy(fullfilepath_in, filebase);
	strcat(fullfilepath_in, thisprojectfile_in);
	printf("fullpath=[%s] \n", fullfilepath_in); // for debugging your filename
	ifp = fopen(fullfilepath_in, "r");
	if (ifp == NULL)
	{
		printf("cannot open file_in");
		system("PAUSE");
		exit(4);
	}

	FILE* ofp;
// create a place where all the pieces will be concatenated
	char *fullfilepath_out = (char *)malloc(30);
// allow for different names per project
	char *thisprojectfile_out = (char *)malloc(14);
	strcpy(thisprojectfile_out, "project3_out.txt");

	strcpy(fullfilepath_out, filebase);
	strcat(fullfilepath_out, thisprojectfile_out);
	printf("fullpath=[%s] \n", fullfilepath_out); // for debugging your filename
	ofp = fopen(fullfilepath_out, "w");
	if (ofp == NULL)
	{
		printf("cannot open file_out");
		system("PAUSE");
		exit(4);
	}

/* gathering and organization of input data */
	A = createMatrix();
	B = createMatrix();
	C = createMatrix();
	getInputDimensions(ifp);
	fillMatrices(ifp);
	fclose(ifp);

/* generation of auxiliary data, allocate memory for matrix C data */
	int countA = A->rows * A->columns;
	int countB = B->rows * B->columns;
	C->rows = A->rows; // characterize matrix C
	C->columns = B->columns;
	int countC = C->rows * C->columns;
	int thread_count = A->rows * countB;
	element *tempC = malloc(sizeof(element) * countC);
	C->matrix = tempC;
	int z = 0;
	for(z; z < countC; z++) { // initialize all data within matrix C to 0

		C->matrix[z].c.row = 0;
		C->matrix[z].c.column = 0;
		C->matrix[z].value = 0;
	}
	mutex_array = malloc(sizeof(sem_t) * countC); // allocate memory for semaphore array
	pthread_t *thread_handles = malloc(sizeof(pthread_t) * thread_count); // allocate memory for threads
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	for(i; i < countC; i++) {

		sem_init(&mutex_array[i], 0, 1); // initialize semaphores with int value 1, shared by threads
	}

/* print matrix A */
	i = 0;
	printf("\nMatrix A\n");
	while(i < countA) {

		printf("%d  ", A->matrix[i].value);
		i++;
		if(i % A->columns == 0) printf("\n");
	};

/* print matrix B */
	i = 0;
	printf("\nMatrix B\n");
	while(i < countB) {

		printf("%d  ", B->matrix[i].value);
		i++;
		if(i % B->columns == 0) printf("\n");
	}
	printf("\n");

//	initialize a data packet pointer to hold data for EACH THREAD
//	embed for() loops to create and join a thread for each multiplication operation to be performed
//	(# threads) = (# mult operations) = (# rows in A) * (# elements in B)
	i = 0;
	packet *p = malloc(sizeof(packet));
	p->a = 0;
	p->b = 0;
	p->c.row = 0;
	p->c.column = 0;
printf("A->rows = %d	A->columns = %d\nB->rows = %d	B->columns = %d\nC->rows = %d	C->columns = %d\n", A->rows, A->columns, B->rows, B->columns, C->rows, C->columns);
	for(k; k < C->rows; k++) {

		j = 0;
		for(j; j < C->columns; j++) {

			l = 0;
			p->c.row = k;
			p->c.column = j;
			for(l; l < A->columns; l++) {

				p->a = A->matrix[(p->c.row * A->columns) + l].value;
				p->b = B->matrix[(p->c.column) + (l * B->columns)].value;
				pthread_create(&thread_handles[i], NULL, getProduct, (void *)p);
				pthread_join(thread_handles[i], NULL);
				i++;
			}
		}
	}

/* print matrix C */
	i = 0;	
	printf("\nMatrix C\n");
	while(i < countC) {

		fprintf(ofp, "%d", C->matrix[i].value);
		printf("%d  ", C->matrix[i].value);
		i++;
		if (i % C->columns == 0) {

			fprintf(ofp, "\n");
		} else {

			fprintf(ofp, " ");
		}
		if(i % C->columns == 0) printf("\n");
	}
	printf("\n");
	fclose(ofp);
	system("PAUSE");

	return 0;
}

//	creates and returns new matrix with initialized members
matrix *createMatrix() {

	matrix *m = malloc(sizeof(matrix));
	m->rows = 0;
	m->columns = 0;
	m->matrix = NULL;
	return m;
}

//	reads file and determines dimensions of both matrices
//	places sets .rows and .columns value in matrix A and matrix B
void getInputDimensions(FILE *f) {

	int c;
	int count = 0;

/* get dimensions of matrix A */
	while(1) {

		c = fgetc(f);
		if(c == 42) break; //asterisk detected
		else if((c == 9) || (c == 32)) {} //tab or space detected
		else if(c == 10) { //new line detected

			A->rows++;
		} else if((c > 47) && (c < 58)) { //digit detected

			count++;
		} else { //invalid character

			printf("Invalid matrix.\n");
			exit(0);
		}
	}
	if(count % A->rows != 0) { //check for invalid dimensions

		printf("Invalid matrix.\n");
		exit(0);
	}
	A->columns = count / A->rows; //calculate # columns
	count = 0;
	while(fgetc(f) == 42) {} //pass the row of asterisks

/* get dimensions of matrix B */
	while(1) {

		c = fgetc(f);
		if (c == EOF) {

			B->rows++;
			break;
		}
		else if((c == 9) || (c == 32)) {} //tab or space detected
		else if(c == 10) { //new line detected

			B->rows++;
		} else if((c > 47) && (c < 58)) { //digit detected

			count++;
		} else { //invalid character

			printf("Invalid matrix.\n");
			exit(0);
		}
	}
	if(count % B->rows != 0) { //check for invalid dimensions

		printf("Invalid matrix.\n");
		exit(0);
	}
	B->columns = count / B->rows; //calculate # columns
}

//	can only be called once dimensions of matrix A and matrix B are known
//	creates and fills two arrays of elements with the matrix data in file
//	points matrix A and matrix B 'matrix' member at these two arrays
void fillMatrices(FILE *f) {

	int c = fseek(f, 0, SEEK_SET);
	int i = 0;
	int row = 0;
	int column = 0;
	int countA = A->rows * A->columns;
	int countB = B->rows * B->columns;
	element *tempA = malloc(sizeof(element) * countA);
	element *tempB = malloc(sizeof(element) * countB);	
	
/* fill matrix A */
	while(i < countA) {

		c = fgetc(f);
		if(c == 10) { //new line detected

			row++;
			column = 0;
		} else if((c > 47) && (c < 58)) { //digit detected

			tempA[i].value = c % 48;
			tempA[i].c.row = row;
			tempA[i].c.column = column;
			column++;
			i++;
		} else {} //tab or space detected
	}
	c = fgetc(f); //pass the line break at end of matrix A
	while(fgetc(f) == 42) {} //pass the row of asterisks
	i = 0;
	row = 0;
	column = 0;

/* fill matrix B */
	while(i < countB) {

		c = fgetc(f);
		if(c == 10) { //new line detected

			row++;
			column = 0;
		} else if((c > 47) && (c < 58)) { //digit detected

			tempB[i].value = c % 48;
			tempB[i].c.row = row;
			tempB[i].c.column = column;
			column++;
			i++;
		} else {} //tab or space detected
	}

/* put element matrices into matrix structures */	
	A->matrix = tempA;
	B->matrix = tempB;		
}

// computes the product of the two values in data packet
// determines the matrix C element index
// sets all member values in matrix C element
void *getProduct(void *p) {

	int product = 0;	
	packet *pack;
	pack = malloc(sizeof(packet));
	pack = (packet *)p;
	product = pack->a * pack->b;
printf("%d\n", product);
	int index = (pack->c.row * C->columns) + pack->c.column;
	sem_wait(&mutex_array[index]);
		
	C->matrix[index].value += product;
	C->matrix[index].c.row = pack->c.row;
	C->matrix[index].c.column = pack->c.column;
	
	sem_post(&mutex_array[index]);
}
