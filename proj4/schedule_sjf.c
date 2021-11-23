# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# include "task.h"
# include "list.h"
# include "cpu.h"
# include "schedulers.h"

struct node *head = NULL; 

//for calculating
int time = 0;		//current time.
int tsk_cnt = 0;	//task count.
int tt_wt_time = 0;	//total waiting time.
int tt_rsp_time = 0;	//total response time.
int tt_ta_time = 0;	//total turnaround time.
int tid_value = 0;	//task identifier
void add(char *name, int priority, int burst) {
	Task *tsk;
	tsk = (Task *) malloc (sizeof(Task));
	//avoid racing conditions
	tsk -> tid = __sync_fetch_and_add(&tid_value, 1);
	tsk -> name = (char *) malloc (sizeof(char) * (1 + strlen(name)));
	strcpy(tsk -> name, name);
	tsk -> priority = priority;
	tsk -> burst = burst;

	//for calculating
	tsk -> arv_time = time;	//arrival time.
	tsk -> wt_time = 0;	//waiting time.
	tsk -> last_exe_time = time;	//last execution time.
	tsk -> rsp_time = 0;	//response time.
	tsk -> ta_time = 0;	//turnaround time.
	insert(&head, tsk);	//add to list
}

void next_tsk() {
	if (head == NULL) return;
	
	struct node *cur = head;
	struct node *dex = head -> next;
	while (dex != NULL)	//sjf next
	{
		if(dex->task->burst <= cur->task->burst) cur = dex;
		dex = dex -> next;
	}
	Task *tsk = cur -> task;
	run(tsk, tsk -> burst);	//execute
	delete(&head, tsk);	//remove from list
	time += tsk -> burst;	//current time

	//for calculating
	//last waiting time.
	int last_wt_time = time - tsk -> last_exe_time - tsk -> burst;
	//waiting time.	
	tsk -> wt_time += last_wt_time;
	//response time.	
	if(tsk->last_exe_time==tsk->arv_time)//first exe
		tsk -> rsp_time = last_wt_time;
	//last execution time.	
	tsk -> last_exe_time = time;
	//turnaround time.
	tsk -> ta_time = time - tsk -> arv_time;
	
	//total data
	tsk_cnt += 1;
	tt_wt_time += tsk -> wt_time;
	tt_rsp_time += tsk -> rsp_time;
	tt_ta_time += tsk -> ta_time;

	//free
	free(tsk -> name);
	free(tsk);
}

void print() {
	printf("\nTotal %d tasks.\n", tsk_cnt);
	double avg_wt_time = 1.0 * tt_wt_time / tsk_cnt;
	double avg_rsp_time = 1.0 * tt_rsp_time / tsk_cnt;
	double avg_ta_time = 1.0 * tt_ta_time / tsk_cnt;
	printf("Average Waiting Time: %.6lf\n", avg_wt_time);
	printf("Average Response Time: %.6lf\n", avg_rsp_time);
	printf("Average Turnaround Time: %.6lf\n", avg_ta_time);
}

void schedule() {
	while (head != NULL)
		next_tsk();
	print();
}
//make sjf
//./sjf schedule.txt
