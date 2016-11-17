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
    size_t len = 0;
    ssize_t line_len = 0;

	while (line_len != -1) {
    	line_len = getline(&line, &len, fp);
    	num_ps++;
    }
    num_ps--;

    rewind(fp);

    if (num_ps == 0) {
    	printf("No processes in file\n");
		printf("CPU usage : 0%%\n");
		printf("Average TAT : 0.0\n");
		printf("Average normalized TAT : 0.0\n");	
		exit(0);
    }
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
	int cur_running = 0, num_ready = 0;
	struct Process *ready_queue[num_ps];
	struct Process *p = NULL;
	float CPU_time = 0.0, sum_TAT = 0.0, sum_nTAT = 0.0;
	int time, i;

	for (i = 0; i < num_ps; i++) {
		ready_queue[i] = NULL;
	}

	// keep running until we've run all processes
	for (time = 0; cur_running < num_ps; time++) {
		// add process to ready queue
		while (num_ready < num_ps && ps[num_ready].arrival_time == time) {
			ready_queue[num_ready] = &ps[num_ready];
			num_ready++;
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
			printf("<time %d> process %d is running\n", time, p->pid);
		} 

		// if process is done running, update TAT and nTAT
		else {
			printf("<time %d> process %d is finished!\n", time, p->pid);
			sum_TAT += (time - p->arrival_time + 1.0);
			sum_nTAT += (time - p->arrival_time + 1.0) / p->service_time;

			p = NULL;
			cur_running++;
		}
	}

	printf("\nCPU usage : %.2f %%\n", (CPU_time / time) * 100);
	printf("Average TAT : %.2f\n", (sum_TAT / num_ps));
	printf("Average normalized TAT : %.2f\n", (sum_nTAT / num_ps));	

	return;
}

//TODO: fix makefile for red black tree
void CFS_loop(struct Process *ps, int num_ps) {
	rb_red_blk_tree *ready_queue = RBTreeCreate(*compare_vruntime,
			     dest_node,
			     dest_node,
			     NULL,
			     NULL);

	int num_ready = 0, num_finished = 0;
	struct Process *p;
	int ready_weights = 0;
	int weight, p_time, timeslice, vruntime;
	int *vrp = NULL;
	float CPU_time = 0.0, sum_TAT = 0.0, sum_nTAT = 0.0;
	int time;
	bool isRunning = false;

	for (time = 0; num_finished < num_ps; time++) {
		// add process to ready queue
		// printf("TIME %d\n", time);
		while (num_ready < num_ps && ps[num_ready].arrival_time == time) {
			// printf("%d arrived\n", ps[num_ready].pid);
			vruntime = 0;
		    vrp=(int*) malloc(sizeof(int));
		    *vrp=vruntime;
			RBTreeInsert(ready_queue, vrp, &ps[num_ready]);
			ready_weights += prio_to_weight[ps[num_ready].priority + 20];
			num_ready++;
		}

		// if no process running, take first process from ready queue
		if (!isRunning) {
			if (ready_queue->root->left == ready_queue->nil) {
				printf("<time %d> CPU is idle\n", time);
				continue;
			} 

			// pop leftmost node in ready_queue
			// pointer to processes getting overwritten
			p = malloc(sizeof(struct Process));
			p = get_next_process(ready_queue);
			isRunning = true;
			p_time = 0;
			weight = prio_to_weight[p.priority + 20];
			timeslice = (TARGET_LATENCY * weight) / ready_weights;
			printf("pid %d\n", p.pid);
			printf("weight %d ready weight %d\n", weight, ready_weights);
			printf("timeslice %d\n", timeslice);
		}

		p.runtime++;
		p_time++;
		CPU_time++;

		if (p.runtime == p.service_time) {
			printf("<time %d> process %d is finished!\n", time, p.pid);
			sum_TAT += (time - p.arrival_time + 1.0);
			sum_nTAT += (time - p.arrival_time + 1.0) / p.service_time;
			ready_weights -= weight;
			num_finished++;
			printf("finished:%d\n", num_finished);

			isRunning = false;
		}
		else {
			printf("<time %d> process %d is running\n", time, p.pid);
			if (p_time == timeslice) {
				//update virtual runtime
				vruntime = p.runtime * (prio_to_weight[20] / weight);
				printf("%d vrtime %d\n", p.pid, vruntime);
			    vrp=(int*) malloc(sizeof(int));
			    *vrp=vruntime;
				//insert to rb tree
				pp = malloc(sizeof(struct Process));
				pp = p;
				RBTreeInsert(ready_queue, vrp, &pp);
				isRunning = false;
			}
		}
	}

	printf("\nCPU usage : %.2f %%\n", (CPU_time / time) * 100);
	printf("Average TAT : %.2f\n", (sum_TAT / num_ps));
	printf("Average normalized TAT : %.2f\n", (sum_nTAT / num_ps));	

	return;
}

int compare_vruntime(const void *a, const void *b) {
	int vra = (int)a;
	int vrb = (int)b;
   	
   	if (vra > vrb) {
   		return 1;
   	} else {
   		return 0;
   	}
}

void dest_node(void* a) {
	;
}

struct Process get_next_process(rb_red_blk_tree *q) {
	rb_red_blk_node* n = q->root->left;
	struct Process p;

	while (n->left != q->nil) {
		n = n->left;
	}	

	p = *(struct Process *)n->info;
	RBDelete(q, n);
	return p;
}






