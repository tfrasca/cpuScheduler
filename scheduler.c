#include "scheduler.h"


int main (int argc, char *argv[]) {
    FILE *inputFile;
    int num_processes;
    Process *ps;

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

    // Read inputfile
    num_processes = get_num_processes(inputFile);
    ps = malloc(sizeof(Process) * num_processes);
    get_processes(inputFile, ps, num_processes);
    fclose(inputFile);

    // Start scheduler loop
    if (strcmp(argv[2], "FCFS") == 0 ) {
        FCFS_loop(ps, num_processes);
    } else if (strcmp(argv[2], "CFS") == 0 ) {
        CFS_loop(ps, num_processes);
    }

    free(ps);
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

    // Return file pointer to beginning of file
    rewind(fp);

    if (num_ps == 0) {
        printf("No processes in file\n");
        printf("CPU usage : 0.00%%\n");
        printf("Average TAT : 0.00\n");
        printf("Average normalized TAT : 0.00\n");   
        exit(0);
    }

    free(line);
    return num_ps;
}

void get_processes(FILE *fp, Process *ps, int num_ps) {
    int num_match, pid, arrival, service, priority;
    int i, j;

    for (i=0; i < num_ps; i++) {
        num_match = fscanf(fp, "%d %d %d %d\n", &pid, &arrival, &service, &priority);

        // Input validation
        if (num_match != 4 || pid < 0 || arrival < 0 || service < 1 || priority < -20 || priority > 19) {
            fprintf(stderr, "Error: Invalid input file format.\n");
            exit(1);
        }

        // Check pid uniquenss
        for (j=0; j<i; j++) {
            if (pid == ps[j].pid) {
                fprintf(stderr, "Error: Invalid input file format.\n");
                exit(1);
            }
        }

        ps[i].pid = pid;
        ps[i].arrival_time = arrival;
        ps[i].service_time = service;
        ps[i].priority = priority;
        ps[i].runtime = 0;
    }

    // Sort processes by arrival time
    qsort(ps, num_ps, sizeof(Process), sort_arrival);

    return;
}

int sort_arrival (const void * a, const void * b) {
    int Pa = ((Process*)a)->arrival_time;
    int Pb = ((Process*)b)->arrival_time;
    
    if (Pa < Pb) {
        return -1;
    } else {
        return 1;
    }
}

void FCFS_loop(Process *ps, int num_ps) {
    int cur_running = 0, num_ready = 0;
    Process *ready_queue[num_ps];
    Process *p = NULL;
    float CPU_time = 0.0, sum_TAT = 0.0, sum_nTAT = 0.0;
    int time, i;

    for (i = 0; i < num_ps; i++) {
        ready_queue[i] = NULL;
    }

    // Keep running until all processes have run
    for (time = 0; cur_running < num_ps; time++) {
        // Add process to ready queue
        while (num_ready < num_ps && ps[num_ready].arrival_time == time) {
            ready_queue[num_ready] = &ps[num_ready];
            num_ready++;
        }

        // If no process running, take first process from ready queue
        if (!p) {
            p = ready_queue[cur_running];

            // If ready queue is empty, CPU is idle
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

        // If process is done running, update TAT and nTAT
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

void CFS_loop(Process *ps, int num_ps) {
    rb_red_blk_tree *ready_queue = RBTreeCreate(*compare_vruntime,
                                                blank_func, blank_func, NULL, NULL);
    int num_ready = 0, num_finished = 0;
    Process *p = NULL;
    int ready_weights = 0, weight, p_time, timeslice;
    float vruntime;
    float *vrp;
    float CPU_time = 0.0, sum_TAT = 0.0, sum_nTAT = 0.0;
    int time;

    for (time = 0; num_finished < num_ps; time++) {
        // Add process to ready queue
        while (num_ready < num_ps && ps[num_ready].arrival_time == time) {
            vruntime = 0;
            vrp=(float*) malloc(sizeof(float));
            *vrp=vruntime;
            RBTreeInsert(ready_queue, vrp, &ps[num_ready]);
            ready_weights += prio_to_weight[ps[num_ready].priority + 20];
            num_ready++;
        }

        // If no process running, take first process from ready queue
        if (!p) {
            // If ready queue is empty, CPU is idle
            if (ready_queue->root->left == ready_queue->nil) {
                printf("<time %d> CPU is idle\n", time);
                continue;
            } 

            // Pop leftmost node in ready_queue
            p = get_next_process(ready_queue);
            p_time = 0;
            weight = prio_to_weight[p->priority + 20];
            timeslice = ((TARGET_LATENCY * weight) + ready_weights - 1) / ready_weights;
        }

        p->runtime++;
        p_time++;
        CPU_time++;

        if (p->runtime < p->service_time) {
            printf("<time %d> process %d is running\n", time, p->pid);

            // If process has from for its timeslice, insert back into ready queue
            if (p_time == timeslice) {
                vruntime = p->runtime * (prio_to_weight[20] / (float)weight); 
                vrp=(float*) malloc(sizeof(float));
                *vrp=vruntime;
                RBTreeInsert(ready_queue, vrp, p);
                p = NULL;
            }
        }

        // If process is done running, update TAT and nTAT
        else {
            printf("<time %d> process %d is finished!\n", time, p->pid);
            sum_TAT += (time - p->arrival_time + 1.0);
            sum_nTAT += (time - p->arrival_time + 1.0) / p->service_time;
            ready_weights -= weight;
            num_finished++;
            p = NULL;
        }
    }

    printf("\nCPU usage : %.2f %%\n", (CPU_time / time) * 100);
    printf("Average TAT : %.2f\n", (sum_TAT / num_ps));
    printf("Average normalized TAT : %.2f\n", (sum_nTAT / num_ps)); 
    
    RBTreeDestroy(ready_queue);
    return;
}

Process *get_next_process(rb_red_blk_tree *q) {
    rb_red_blk_node* n = q->root->left;
    Process *p;

    while (n->left != q->nil) {
        n = n->left;
    }   

    p = (Process *)n->info;

    free(n->key);
    RBDelete(q, n);
    return p;
}

int compare_vruntime(const void *a, const void *b) {
    float vra = *(float*)a;
    float vrb = *(float*)b;
    
    if (vra > vrb) {
        return 1;
    } else {
        return 0;
    }
}

void blank_func(void *a) {
    ;
}



