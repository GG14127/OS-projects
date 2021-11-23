#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

int n;
int *array;
int *rst;
//quick sort for sorting_thread
int qsort_compare(const int*a, const int *b)
{
	return *a-*b;
}

void* qsorting(int *arg)
{
	if(arg[1]-arg[0]<0) return NULL;//invalid index
	qsort(array+arg[0],arg[1]-arg[0]+1, sizeof(int), qsort_compare);
	return NULL;
}
//mergesort for merging_thread
void* msorting(int *arg)
{
	int rst_dex=arg[0];
	int dex0=arg[0];
	int dex1=arg[1]+1;
	while(dex0<=arg[1]&&dex1<=arg[2])
	{
		if(array[dex0]<=array[dex1]) 
		{
			rst[rst_dex]=array[dex0];
			rst_dex++;
			dex0++;
		}
		else
		{
			rst[rst_dex]=array[dex1];
			rst_dex++;
			dex1++;
		}
	}
	while(dex0<=arg[1])
	{
		rst[rst_dex]=array[dex0];
		rst_dex++;
		dex0++;
	}
	while(dex1<=arg[2])
	{
		rst[rst_dex]=array[dex1];
		rst_dex++;
		dex1++;
	}
}

int main(void)
{
	//for random number
	srand((unsigned)time(NULL));
	int error=0;
	printf("input the array length n (0<=n<=10000): ");
	scanf("%d", &n);
	
	if(n<0||n>10000)
	{
		printf("ERROR:n is out of range\n");
		exit(1);
	}
	
	
	array=(int *)malloc(n*sizeof(int));
	rst=(int *)malloc(n*sizeof(int));
	
	char random[2];
	printf("generate the random elements?(Y/N):\n");
	scanf("%s", random);
	if(random[0]=='Y')	//generate random number
	{
		for (int i = 0; i < n; ++ i)
		{
			array[i] = rand() % 1000;
			printf("%d ",array[i]);
		}
		printf("\n");
	}
	else	//read in array
	{
		printf("input the array elements:\n");
		for(int i=0;i<n;i++)
			scanf("%d",&array[i]);
	}
	
	//Range parameter for sorting
	int parameters0[2];
	int parameters1[2];
	parameters0[0]=0;
	parameters0[1]=n/2;
	parameters1[0]=n/2+1;
	parameters1[1]=n-1;
	
	//create 2 sorting_thread
	pthread_t sorting_thread[2];
	error=pthread_create(&sorting_thread[0],NULL,qsorting,&parameters0);
	if(error)
	{
		printf("ERROR:create thread failed\n");
		exit(1);
	}
	error=pthread_create(&sorting_thread[1],NULL,qsorting,&parameters1);
	if(error)
	{
		printf("ERROR:create thread failed\n");
		exit(1);
	}
	error=pthread_join(sorting_thread[0],NULL);
	if(error)
	{
		printf("ERROR:thread join failed\n");
		exit(1);
	}
	error=pthread_join(sorting_thread[1],NULL);
	if(error)
	{
		printf("ERROR:thread join failed\n");
		exit(1);
	}
	
	
	//Range parameter for merging
	int mergeparam[3];
	mergeparam[0]=0;
	mergeparam[1]=n/2;
	mergeparam[2]=n-1;
	
	//create merging_thread
	pthread_t merging_thread;
	error=pthread_create(&merging_thread,NULL,msorting,&mergeparam);
	if(error)
	{
		printf("ERROR:create thread failed\n");
		exit(1);
	}
	error=pthread_join(merging_thread,NULL);
	if(error)
	{
		printf("ERROR:thread join failed\n");
		exit(1);
	}
	
	printf("after sorting: \n");
	for(int i=0;i<n;i++)
		printf("%d ",rst[i]);
	printf("\n");
	
	//free
	free(array);
	free(rst);
	
	return 0;
	
}
//gcc multithreaded_sorting.c -o ./sort -g -lpthread
//./sort

