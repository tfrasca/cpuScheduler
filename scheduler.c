#include "scheduler.h"


int main (int argc, char *argv[]) {
	FILE *inputFile;
	int num_processes;
	struct Process *ps;

	if (argc != 3) {
		fprintf(stderr, "Error: ./scheduler expects 2 arguments.\n");
		exit(1);
	}

	inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Error: Invalid input file.\n");
        exit(1);
    }

	if (strcmp(argv[2], "FCFS") != 0 && strcmp(argv[2], "CFS") != 0) {
		fprintf(stderr, "Error: ./scheduler only runs in FCFS mode or CFS mode.\n");
		exit(1);
	}

	//read inputfile
	num_processes = get_num_processes(inputFile);
	ps = get_processes(inputFile, num_processes, argv[2]);

	if (strcmp(argv[2], "FCFS") == 0 ) {
		FCFS_loop(ps, num_processes);
	} else if (strcmp(argv[2], "CFS") == 0 ) {
		CFS_loop(ps, num_processes);
	}

	return 0;
}

int get_num_processes(FILE *fp) {
	int num_ps = 0;
	char *line = NULL;
    char *word;
    size_t len = 0;
    ssize_t line_len = 0;

	while (line_len != -1) {
    	line_len = getline(&line, &len, fp);
    	num_ps++;
    }
    num_ps--;

    rewind(fp);
	return num_ps;
}

struct Process *get_processes(FILE *fp, int num_ps, char *mode) {
	struct Process *ps;
    int num_match = 0;
    int i;

    ps = malloc(sizeof(struct Process) * num_ps);

    for (i=0; i < num_ps; i++) {
	    num_match = fscanf(fp, "%d %d %d %d\n", &ps[i].pid, &ps[i].arrival_time, &ps[i].service_time, &ps[i].priority);
	    if (num_match != 4) {
	        fprintf(stderr, "Error: Invalid input file format.\n");
	        exit(1);
	    }
	    ps[i].runtime = 0;

	  //   if (strcmp(mode, "CFS") == 0) {
	  //   	p.timeslice = (TARGET_LATENCY * weight);
			// p.vruntime = 0;
	  //   }
    }

    qsort(ps, num_ps, sizeof(struct Process), sort_arrival);

    return ps;
}

int sort_arrival (const void * a, const void * b) {
	int Pa = ((struct Process*)a)->arrival_time;
	int Pb = ((struct Process*)b)->arrival_time;
   	
   	if (Pa < Pb) {
   		return -1;
   	} else {
   		return 1;
   	}
}

void FCFS_loop(struct Process *ps, int num_ps) {
	int cur_process = 0;
	int cur_running = 0;
	int num_ready = 0;
	int time;
	struct Process *ready_queue[num_ps];
	struct Process *p = NULL;
	float CPU_time = 0.0;
	float sum_TAT = 0.0;
	float sum_nTAT = 0.0;
	int i;

	for (i = 0; i < num_ps; i++) {
		ready_queue[i] = NULL;
	}

	for (time = 0; 1; time++) {
		// add process to ready queue
		while (cur_process < num_ps && ps[cur_process].arrival_time == time) {
			ready_queue[num_ready++] = &ps[cur_process++];
		}

		// if no process running, take first process from ready queue
		if (!p) {
			p = ready_queue[cur_running];

			//if ready queue is empty, CPU is idle
			if (!p) {
				printf("<time %d> CPU is idle\n", time);
				continue;
			}
		}

		p->runtime++;
		CPU_time++;

		if (p->runtime < p->service_time) {
			printf("<time %d> process %d is running\n", time, ready_queue[cur_running]->pid);
		} 

		// if process is done running, update TAT and nTAT
		else {
			printf("<time %d> process %d is finished!\n", time, p->pid);
			sum_TAT += (time - p->arrival_time + 1.0);
			sum_nTAT += (time - p->arrival_time + 1.0) / p->service_time;

			p = NULL;
			cur_running++;

			//if we've finished running all processes
			if (cur_running == num_ps) {
				break;
			}
		}
	}

	printf("\nCPU usage : %.2f %%\n", (CPU_time / time) * 100);
	printf("Average TAT : %.2f\n", (sum_TAT / num_ps));
	printf("Average normalized TAT : %.2f\n", (sum_nTAT / num_ps));

	return;
}


void CFS_loop(struct Process *ps, int num_ps) {
	return;
}










