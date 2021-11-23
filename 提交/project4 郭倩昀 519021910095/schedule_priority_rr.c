# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# include "task.h"
# include "list.h"
# include "cpu.h"
# include "schedulers.h"

struct node *head[MAX_PRIORITY - MIN_PRIORITY + 1] = {}; 

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
	insert(&head[priority- MIN_PRIORITY], tsk);	//add to list
}

void next_tsk(int priority) {
	if (head[priority- MIN_PRIORITY] == NULL) return;
	
	struct node *cur = head[priority- MIN_PRIORITY];
	while (cur -> next != NULL) //next priority_rr last one
		cur = cur -> next;
	Task *tsk = cur -> task;
	if(tsk -> burst <= QUANTUM)	//less than QUANTUM finish
	{	run(tsk, tsk -> burst);	//execute burst time
		delete(&head[priority- MIN_PRIORITY], tsk);	//remove from list
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
	else	//more than QUANTUM
	{
		run(tsk, QUANTUM);	//execute QUANTUM
		delete(&head[priority- MIN_PRIORITY], tsk);
		time +=	QUANTUM;	//update current time
		//update task info for calculating
		tsk -> burst = tsk -> burst - QUANTUM;
		//last waiting time.
		int last_wt_time = time - tsk -> last_exe_time - QUANTUM;
		//waiting time.	
		tsk -> wt_time += last_wt_time;
		if(tsk->last_exe_time==tsk->arv_time)//first exe
			tsk -> rsp_time = last_wt_time;
		//last execution time.	
		tsk -> last_exe_time = time;
		
		insert(&head[priority- MIN_PRIORITY], tsk);	//add to list
	}
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
	for(int i = MAX_PRIORITY; i >= MIN_PRIORITY ; --i)
	{	
		while (head[i- MIN_PRIORITY]!= NULL)
		next_tsk(i);
	}
	print();
}
//make priority_rr
//./priority_rr schedule.txt
