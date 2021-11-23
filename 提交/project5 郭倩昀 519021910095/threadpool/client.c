/**
 * Example client program that uses thread pool.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "threadpool.h"

struct data
{
    int a;
    int b;
};

void add(void *param)
{
    struct data *temp;
    temp = (struct data*)param;

    printf("I add two values %d and %d result = %d\n",temp->a, temp->b, temp->a + temp->b);
}

int main(void)
{
	
    // create some work to do
    srand((unsigned)time(NULL));
	struct data work[10];
	for (int i = 0; i < 10; ++ i) {
		work[i].a = rand() % 100;
		work[i].b = rand() % 100;
	}


    // initialize the thread pool
    pool_init();

    // submit the work to the queue
	for (int i = 0; i < 10; ++ i) {
		static int err;
		err = pool_submit(&add, &work[i]);
		if (err) 
		fprintf(stderr, "Submit work %d error because the task queue is full.\n", i);
	}

    sleep(3);

    pool_shutdown();

    return 0;
}
