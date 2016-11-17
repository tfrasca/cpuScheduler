#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// pid arrival_time service_time priority 

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: ./processes <num_processes> <output_file>\n");
		exit(1);
	}

	int num_p = atoi(argv[1]);
	int array[num_p];
	int i;
	int pid;
	int arrival_time;
	int service_time;
	int priority;
	int temp;
	int randomIndex;
	time_t t;
	FILE *fp;

	srand((unsigned) time(&t));
	fp = fopen(argv[2], "w");

	for (i = 0; i < num_p; i++) {     // fill array
	    array[i] = i;
	}

	for (i = 0; i < num_p; i++) {    // shuffle array
	    temp = array[i];
	    randomIndex = rand() % num_p;

	    array[i] = array[randomIndex];
	    array[randomIndex] = temp;
	}

	for (i=0; i < num_p; i++) {
		pid = array[i];
		arrival_time = rand() % 50;
		service_time = (rand() % 20) + 1;
		priority = (rand() % 40) - 20;

		fprintf(fp, "%d %d %d %d\n", pid, arrival_time, service_time, priority);
	}

	fclose(fp);

	return 0;
}
