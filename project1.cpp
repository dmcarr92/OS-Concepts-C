// Carr_Dillon_CS311_Project1.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "time_functions.h"
#include "stdafx.h"


void char_by_char(char r_fname[], char w_fname[]);
void line_by_line(char r_fname[], char w_fname[]);


int main() {

	int mode;
	char r_fname[32];
	char w_fname[32];

	printf("Enter the name of a file you'd like to read from.\n\n");
	scanf("%s", r_fname);
	printf("Enter the name of a file you'd like to write to.\n\n");
	scanf("%s", w_fname);
	printf("Enter a 0 to run in character-by-character mode.\n\nEnter a 1 to run in line-by-line mode.\n\n");
	scanf("%d", &mode);
	if (mode == 0) {

		char_by_char(r_fname, w_fname);
	}
	else if (mode == 1) {

		line_by_line(r_fname, w_fname);
	}
	else {

		printf("Invalid mode selected, please enter a 0 to run in character-by-character mode, or a 1 to run in line-by-line mode.\n\n");
		scanf("%d", &mode);
		if (mode == 0) {

			char_by_char(r_fname, w_fname);
		}
		else {

			line_by_line(r_fname, w_fname);
		}
	}
	return 0;
}


void char_by_char(char r_fname[], char w_fname[]) {

	int c;
	FILE *read_from = fopen(r_fname, "r");
	FILE *write_to = fopen(w_fname, "w");
	if (read_from == NULL) {

		printf("Problem opening read file, please re-enter file name\n\n");
		scanf("%s", r_fname);
		read_from = fopen(r_fname, "r");
	}
	start_timing();
	while ((c = fgetc(read_from)) != EOF) {

		fputc(c, write_to);
	}
	stop_timing();
	fclose(read_from);
	fclose(write_to);
	printf("Wall-clock time: %16f ms\n\n", get_wall_clock_diff());
	printf("CPU time: %16f ms\n\n", get_CPU_time_diff());
	return;
}


void line_by_line(char r_fname[], char w_fname[]) {

	char line[82];
	FILE *read_from = fopen(r_fname, "r");
	FILE *write_to = fopen(w_fname, "w");
	if (read_from == NULL) {

		printf("Problem opening read file, please re-enter file name\n\n");
		scanf("%s", r_fname);
		read_from = fopen(r_fname, "r");
	}
	start_timing();
	while (fgets(line, 82, read_from) != NULL) {

		fputs(line, write_to);
	}
	stop_timing();
	fclose(read_from);
	fclose(write_to);
	printf("Wall-clock time: %16f ms\n\n", get_wall_clock_diff());
	printf("CPU time: %16f ms\n\n", get_CPU_time_diff());
	return;
}