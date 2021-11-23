/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3

#define TRUE 1

// this represents work that has to be 
// completed by a thread in the pool
typedef struct 
{
    void (*function)(void *p);
    void *data;
}
task;

// the work queue
struct queue_node{	//linked list
	task worktodo;
	struct queue_node *next;
};
struct queue_node *head,*tail;


pthread_t bee[NUMBER_OF_THREADS];	// the worker bee
pthread_mutex_t queue_mutex;		//mutex
sem_t sem;				//semaphore
int shutdown;

// insert a task into the queue
// returns 0 if successful or 1 otherwise, 
int enqueue(task t) 
{
	tail->next=(struct queue_node *) malloc (sizeof(struct queue_node));
	if(tail->next==NULL) return 1;	//allocation error occurs
	tail=tail->next;				//add to the end
	tail->worktodo=t;
    return 0;
}

// remove a task from the queue
task dequeue() 
{
	if(head==tail)					//queue is empty
	{
		fprintf(stderr,"ERROR:no work to do\n");
		exit(1);
	}
	else
	{
		struct queue_node *tmp;		//remove from the head
		tmp=head;
		head=head->next;
		free(tmp);
	}
    return head->worktodo;
}

// the worker thread in the thread pool
void *worker(void *param)			//exe by each thread in the pool
{
	task tsk;
	while(TRUE)
	{
		sem_wait(&sem);				//notifying a waiting thread
		if(shutdown) break;
		pthread_mutex_lock(&queue_mutex);//avoid race conditions when accessing queue
		tsk=dequeue();
		pthread_mutex_unlock(&queue_mutex);
		execute(tsk.function, tsk.data);// execute the task
	}
    pthread_exit(0);
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *p), void *p)
{
	task tsk;		//place function and data
	tsk.function=somefunction;
	tsk.data=p;
	pthread_mutex_lock(&queue_mutex);//avoid race conditions when accessing queue
	int rst=enqueue(tsk);
	pthread_mutex_unlock(&queue_mutex);
	if(rst==0) sem_post(&sem);
    return rst;
}

// initialize the thread pool
void pool_init(void)
{
	int err;
	shutdown=0;
	head = (struct queue_node *) malloc (sizeof(struct queue_node));
	if(head==NULL)
	{
		fprintf(stderr,"ERROR:queue init error\n");
		exit(1);
	}
	head->next=NULL;
	tail=head;
	
	err=pthread_mutex_init(&queue_mutex,NULL);	//initialize mutex
	if(err)
	{
		fprintf(stderr,"ERROR:pthread mutex init error\n");
		exit(1);
	}
	err=sem_init(&sem,0,0);						//initialize semaphore
	if(err)
	{
		fprintf(stderr,"ERROR:semaphore init error\n");
		exit(1);
	}
	for(int i=0;i<NUMBER_OF_THREADS;i++)		//pthread create
	{
		err=pthread_create(&bee[i],NULL,worker,NULL);
		if(err)
		{
			fprintf(stderr,"ERROR:pthread create error\n");
			exit(1);
		}
	}
	fprintf(stdout,"Pthread create successfully\n");
}

// shutdown the thread pool
void pool_shutdown(void)
{
	int err;
	shutdown=1;
	for(int i=0;i<NUMBER_OF_THREADS;i++)		//cancel each worker thread
	{
		sem_post(&sem);
	}
	for(int i=0;i<NUMBER_OF_THREADS;i++)		//wait for each thread to terminate
	{
		err=pthread_join(bee[i],NULL);
		if(err)
		{
			fprintf(stderr,"ERROR:pthread join error\n");
			exit(1);
		}
	}
	fprintf(stdout,"Pthread join successfully\n");
	err=pthread_mutex_destroy(&queue_mutex);	//destroy mutex
    if(err)
	{
		fprintf(stderr,"ERROR:pthread mutex destroy error\n");
		exit(1);
	}
	err=sem_destroy(&sem);						//destroy semaphore
	if(err)
	{
		fprintf(stderr,"ERROR:semaphore destroy error\n");
		exit(1);
	}
}
