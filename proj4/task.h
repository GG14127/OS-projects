
#ifndef TASK_H
#define TASK_H


// representation of a task
typedef struct task {
	char *name;
	int tid;
	int priority;
	int burst;
    //for calculating
	int arv_time;	//arrival time.
	int wt_time;	//waiting time.
	int last_exe_time;	//last execution time.
	int rsp_time;	//response time.
	int ta_time;	//turnaround time.
} Task;

#endif
