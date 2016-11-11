#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TARGET_LATENCY 1000

struct Process {
	int pid;
	int arrival_time;
	int service_time;
	int priority;

	int runtime;
	int timeslice;
	float vruntime;
};

//read from file
int get_num_processes(FILE *fp);
struct Process *get_processes(FILE *fp, int num_ps, char *mode);
int sort_arrival (const void * a, const void * b);

//FCFS loop
void FCFS_loop(struct Process *ps, int num_ps);

//CFS loop
void CFS_loop(struct Process *ps, int num_ps);
