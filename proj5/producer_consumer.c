#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "buffer.h"

#define TRUE 1
#define RANDOM_TIME_BASE 10
int terminate;
pthread_mutex_t mutex;//for accessing buffer
sem_t empty, full;

void *producer(void *param);

void *consumer(void *param);

int main(int argc, char *argv[]){
	srand((unsigned)time(NULL));
	
	terminate=0;
	pthread_t *producer_t, *consumer_t;
	int time, producer_number, consumer_number;
	static int err;
	
//get command line
	if(argc!=4)
	{
		fprintf(stderr,"ERROR:invalid arguments\n");
		exit(1);
	}
	time=atoi(argv[1]);
	producer_number=atoi(argv[2]);
	consumer_number=atoi(argv[3]);
	
//initialize the buffer
	buffer_init();
	
	//initialize the mutex lock
	err=pthread_mutex_init(&mutex, NULL);
	if(err)
	{
		fprintf(stderr,"ERROR:pthread mutex init create error\n");
		exit(1);
	}
	
	//initialize the semaphore
	err=sem_init(&empty, 0, BUFFER_SIZE);//empty initialize to n
	if(err)
	{
		fprintf(stderr,"ERROR:semaphore init create error\n");
		exit(1);
	}
	err=sem_init(&full, 0, 0);//full initialize to 0
	if(err)
	{
		fprintf(stderr,"ERROR:semaphore init create error\n");
		exit(1);
	}
	
//create producer threads
	producer_t = (pthread_t*)malloc(sizeof(pthread_t)*producer_number);
	for(int i=0;i<producer_number;i++)
	{
		pthread_create(&producer_t[i],NULL,&producer,NULL);
	}
	
//create consumer threads
	consumer_t = (pthread_t*)malloc(sizeof(pthread_t)*consumer_number);
	for(int i=0;i<consumer_number;i++)
	{
		pthread_create(&consumer_t[i],NULL,&consumer,NULL);
	}
	fprintf(stdout,"Pthread create successfully\n");
//sleep
	sleep(time);
	
//terminate
	terminate=1;
	//cancel each thread
	for(int i=0;i<producer_number;i++)
	{
		sem_post(&empty);
	}
	for(int i=0;i<consumer_number;i++)
	{
		sem_post(&full);
	}
	//pthread join
	for(int i=0;i<producer_number;i++)
	{
		err=pthread_join(producer_t[i],NULL);
		if(err)
		{
			fprintf(stderr,"ERROR:pthread join error\n");
			exit(1);
		}
	}
	for(int i=0;i<consumer_number;i++)
	{
		err=pthread_join(consumer_t[i],NULL);
		if(err)
		{
			fprintf(stderr,"ERROR:pthread join error\n");
			exit(1);
		}
	}
	fprintf(stdout,"Pthread join successfully\n");
	
	//destroy mutex
	err=pthread_mutex_destroy(&mutex);	
    if(err)
	{
		fprintf(stderr,"ERROR:pthread mutex destroy error\n");
		exit(1);
	}
	
	//destroy semaphore
	err=sem_destroy(&empty);						
	if(err)
	{
		fprintf(stderr,"ERROR:semaphore destroy error\n");
		exit(1);
	}
	err=sem_destroy(&full);						
	if(err)
	{
		fprintf(stderr,"ERROR:semaphore destroy error\n");
		exit(1);
	}
	free(producer_t);
	free(consumer_t);
	
	return 0;
}

void *producer(void *param)
{
	buffer_item item;
	while(TRUE)
	{
		//sleep for a random period of time
		int random_time;
		random_time=rand()%RANDOM_TIME_BASE;
		sleep(random_time);
		
		//generate a random number
		item=rand();
		
		sem_wait(&empty);
		pthread_mutex_lock(&mutex);
		if(terminate) break;
		if(insert_item(item))
			fprintf(stderr,"ERROR: cannot insert item %d\n", item);
		else
			fprintf(stdout,"Producer produced item %d\n", item);
		pthread_mutex_unlock(&mutex);
		sem_post(&full);
	}
	pthread_mutex_unlock(&mutex);
	pthread_exit(0);
}

void *consumer(void *param)
{
	buffer_item item;
	while(TRUE)
	{
		//sleep for a random period of time
		int random_time;
		random_time=rand()%RANDOM_TIME_BASE;
		sleep(random_time);
		
		sem_wait(&full);
		pthread_mutex_lock(&mutex);
		if(terminate) break;
		if(remove_item(&item))
			fprintf(stderr,"ERROR: cannot remove item %d\n", item);
		else
			fprintf(stdout,"Consumer consumed item %d\n", item);
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
	}
	pthread_mutex_unlock(&mutex);
	pthread_exit(0);
}

//./producer_consumer
