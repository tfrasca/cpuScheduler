#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "rb_tree/red_black_tree.h"
// Taken from http://web.mit.edu/~emin/Desktop/ref_to_emin/www.old/source_code/red_black_tree/index.html

#define TARGET_LATENCY 5

static const int prio_to_weight[40] = {
 /* -20 */     88761,     71755,     56483,     46273,     36291,
 /* -15 */     29154,     23254,     18705,     14949,     11916,
 /* -10 */      9548,      7620,      6100,      4904,      3906,
 /*  -5 */      3121,      2501,      1991,      1586,      1277,
 /*   0 */      1024,       820,       655,       526,       423,
 /*   5 */       335,       272,       215,       172,       137,
 /*  10 */       110,        87,        70,        56,        45,
 /*  15 */        36,        29,        23,        18,        15,
};
// Taken from https://blog.shichao.io/2015/07/22/relationships_among_nice_priority_and_weight_in_linux_kernel.html

typedef struct {
	int pid;
	int arrival_time;
	int service_time;
	int priority;

	int runtime;
} Process;

// read processes from file
int get_num_processes(FILE *fp);
void get_processes(FILE *fp, Process *ps, int num_ps);
int sort_arrival (const void *a, const void *b);

//FCFS loop
void FCFS_loop(Process *ps, int num_ps);

//CFS loop
void CFS_loop(Process *ps, int num_ps);
Process *get_next_process(rb_red_blk_tree *q);
int compare_vruntime(const void *a, const void *b);
void blank_func(void* a);
