# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# define MAX_LINE 500
# define TRUE 1
int resource_num;
int customer_num;


int *available;		// the available amount of each resource
int **maximum;		// the maximum demand of each customer
int **allocation;	// the amount currently allocated to each other
int **need;			// the remaining need of each customer
//int capacity;	// current capacity ? for initialize



void initialize(int argc, char *argv[]);
int parse_inst(char *buf, char *op, int *argn, int *arg);	//Parse the buffer
void update_need(int ** need, int ** maximum, int ** allocation);	//Update the need
int request_resources(int customer_id, int request[]);		//Request the resources
void release_resources(int customer_id, int release[]);		//Release the resources
void print_value(int printop);								//* print current value


int main(int argc, char *argv[]) {

//Initialize the arrays
	initialize(argc, argv);
	
//print initial state
	print_value(0);
	
// Check whether the initial state is safe
	static int err;
	for (int i = 0; i < customer_num; ++ i)
		for (int j = 0; j < resource_num; ++ j)
			if(maximum[i][j] > available[j]) err=1;
	err=0;
	if (err) {
		fprintf(stdout, "  ERROR: Initial state is unsafe\n");
		exit(1);
	}
	
//read in inst
	char buf[MAX_LINE], op[MAX_LINE];
	int *arg = (int *) malloc (sizeof(int) * (1 + resource_num));
	int argn;

	while(TRUE) 
	{
		fprintf(stdout, "Banker >> ");
		fgets(buf, MAX_LINE, stdin);
		
		//Parse the buffer to op and arg
		err = parse_inst(buf, op, &argn, arg);
		if (err) {
			fprintf(stdout, "  ERROR: Invalid instruction\n");
			continue;
		}
		
		//op
		if (strcmp(op, "EXIT") == 0 && argn == 0)//end
			break;
		else if (strcmp(op, "*") == 0 && argn == 0)//print current value
			print_value(2);
		else if (strcmp(op, "RQ") == 0 && argn == resource_num + 1) 
		{
			if (request_resources(arg[0], arg + 1)==-1)	//unsafe
				fprintf(stdout, "  Request command denied.\n");
			else									//safe
				fprintf(stdout, "  Request command accepted.\n");
		}
		else if (strcmp(op, "RL") == 0 && argn == resource_num + 1)//release
			release_resources(arg[0], arg + 1);
		else 
		{
			fprintf(stdout, "  ERROR: Invalid instruction\n");
			continue;
		}
	}
//free
	free(arg);
	free(available);
	for (int i = 0; i < customer_num; ++ i) 
	{
		free(maximum[i]);
		free(allocation[i]);
		free(need[i]);
	}
	free(maximum);
	free(allocation);
	free(need);
	return 0;
}

//Initialize the arrays
void initialize(int argc, char *argv[]) {
	//read in resources
	resource_num = argc - 1;
	if (resource_num == 0) 
	{
		fprintf(stderr, "  ERROR: no resource!\n");
		exit(1);
	}
	//initialize available
	available = (int *) malloc (sizeof(int) * resource_num);
	for (int i = 1; i < argc; ++ i)
		available[i - 1] = atoi(argv[i]);

	//initialize customer maximum
	customer_num = 0;
	int capacity = 100;//default capacity
	maximum = (int **) malloc (sizeof(int *) * capacity);

	//read data from maximum.txt
	FILE *fp = fopen("maximum.txt", "r");
	static int data;
	while(fscanf(fp, "%d", &data)!=EOF)
	{
		//double the array if full
		if (customer_num == capacity) 
		{
			int ** tmp;
			tmp = (int **) malloc (sizeof(int *) * capacity * 2);
			for (int i = 0; i < capacity; ++ i) 
			{
				tmp[i] = (int *) malloc (sizeof(int) * resource_num);
				for (int j = 0; j < resource_num; ++ j)
					tmp[i][j] = maximum[i][j];
				free(maximum[i]);
			}
			free(maximum);
			maximum = tmp;
			capacity*=2;
		}
		
		// read the data
		maximum[customer_num] = (int *) malloc (sizeof(int) * resource_num);
		maximum[customer_num][0] = data;
		for (int i = 1; i < resource_num; ++ i)
		{
			fscanf(fp, ",%d", &data);
			maximum[customer_num][i] = data;
		}
		customer_num ++;
	}
	fclose(fp);
	
	//initialize allocation
	allocation = (int **) malloc (sizeof(int *) * capacity);
	for (int i = 0; i < customer_num; ++ i)
		allocation[i] = (int *) malloc (sizeof(int) * resource_num);
		
	for (int i = 0; i < customer_num; ++ i)
		for (int j = 0; j < resource_num; ++ j) 
			allocation[i][j] = 0;
			
	//initialize need
	need = (int **) malloc (sizeof(int *) * capacity);
	for (int i = 0; i < customer_num; ++ i)
		need[i] = (int *) malloc (sizeof(int) * resource_num);
	update_need(need, maximum, allocation);
}

//Update need
void update_need(int **need, int **maximum, int **allocation)
{
	for (int i = 0; i < customer_num; ++ i)
		for (int j = 0; j < resource_num; ++ j)
			need[i][j] = maximum[i][j] - allocation[i][j];
}

//Parse the buffer
int parse_inst(char *buf, char *op, int *argn, int *arg)
{
	int last_blank=1;
	int tmp=0;
	int opdex = 0;	
	(*argn) = -1;	
	for (int i = 0; buf[i]; ++ i) 
	{
		if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n') 
		{
			if (last_blank) continue;
			last_blank = 1;
			if (*argn != -1) //data
			{
				if (*argn == resource_num + 1) return 1;
				arg[*argn] = tmp;
				tmp = 0;//renew tmp
			}
			(*argn) ++;
		} 
		else 
		{
			last_blank = 0;
			if(*argn == -1) //op
				op[opdex++] = buf[i];
			else 			//data
			{
				if (buf[i]>='0' && buf[i]<='9') tmp = tmp*10+buf[i]-'0';
				else return 1;
			}
		}
	}
	op[opdex] = 0;
	if(!last_blank) //chech buffer end
	{
		if (*argn != -1) 
		{
			if (*argn == resource_num + 1) return 1;				
			arg[*argn] = tmp;
			tmp = 0;//renew tmp
		}
		(*argn) ++;
	}
	return 0;	
}

// Request the resources
int request_resources(int customer_id, int request[])
{
	//pre-check
	for (int i = 0; i < resource_num; ++ i)
		if (request[i] > need[customer_id][i]) 
		{
			fprintf(stdout, "  ERROR: The request is greater than need\n");
			return -1;
		}
	for (int i = 0; i < resource_num; ++ i)
		if (request[i] > available[i]) 
		{
			fprintf(stdout, "  ERROR: Not enough available resources\n");
			return -1;
		}
	
	//grant the request
	int *available_tmp;
	int *is_served;
	available_tmp = (int *) malloc (sizeof(int) * resource_num);
	is_served = (int *) malloc (sizeof(int) * customer_num);
	for (int i = 0; i < customer_num; ++ i)
		is_served[i] = 0;
	for (int i = 0; i < resource_num; ++ i) {
		available_tmp[i] = available[i] - request[i];
		allocation[customer_id][i] += request[i];
	}
	update_need(need, maximum, allocation);
	
	//check
	int safe = 1;
	for (int step = 0; step < customer_num; ++ step) {
		//Find next customer	
		int dex = -1;
		for (int i = 0; i < customer_num; ++ i) 
		{
			if (is_served[i]) continue;
			int flag = 1;
			for (int j = 0; j < resource_num; ++ j)
				if (need[i][j] > available_tmp[j]) 
				{
					flag = 0;
					break;
				}
			if (flag) 
			{
				dex = i;
				break;
			}
		}
		//Not found, unsafe.
		if(dex == -1) {
			safe = 0;
			break;
		}
		//Found, serve the customer.
		is_served[dex] = 1;
		for (int i = 0; i < resource_num; ++ i)
			available_tmp[i] += allocation[dex][i];
	}
	
	//safe
	if (safe) 
	{
		fprintf(stdout, "  Request is granted.\n");
		for (int i = 0; i < resource_num; ++ i)
			available[i] -= request[i];					//grant the request
		free(available_tmp);
		free(is_served);
		return 0;
	} 
	else 
	{
		fprintf(stdout, "  Unsafe state, request CANNOT be granted\n");
		for (int i = 0; i < resource_num; ++ i)
			allocation[customer_id][i] -= request[i];	//take back the resourses
		update_need(need, maximum, allocation);
		free(available_tmp);
		free(is_served);
		return -1;
	}
}

// Release the resources
void release_resources(int customer_id, int release[])
{
	//Pre-Check
	for (int i = 0; i < resource_num; ++ i)
		if (release[i] > allocation[customer_id][i]) 
		{
			fprintf(stdout, "  ERROR: The release is greater than allocation\n");
			return;
		}
		
	//update available and allocation
	for (int i = 0; i < resource_num; ++ i) {
		available[i] += release[i];
		allocation[customer_id][i] -= release[i];
	}
	update_need(need, maximum, allocation);
	fprintf(stdout, "  The resources are released.\n");
	return;
}

//* print current value
//printop 0 available maximum
//printop 1 available maximum allocation
//printop 2 available maximum need
void print_value(int printop)
{
	fprintf(stdout, "Current State: \n");
	fprintf(stdout, "  Customer Number = %d\n  Resource Number = %d\n", customer_num, resource_num);
	
	//available
	fprintf(stdout, "  Available = [");
	for (int i = 0; i < resource_num; ++ i)
		fprintf(stdout, "%d%c%c", available[i], (i == resource_num - 1) ? ']' : ',', (i == resource_num - 1) ? '\n' : ' ');
		
	//maximum
	fprintf(stdout, "  Maximum = \n");
	for (int i = 0; i < customer_num; ++ i) {
		fprintf(stdout, "    [");
		for (int j = 0; j < resource_num; ++ j)
			fprintf(stdout, "%d%c%c", maximum[i][j], (j == resource_num - 1) ? ']' : ',', (j == resource_num - 1) ? '\n' : ' ');
	}

	//allocation
	if (printop >= 1) {
		fprintf(stdout, "  Allocation = \n");
		for (int i = 0; i < customer_num; ++ i) {
			fprintf(stdout, "    [");
			for (int j = 0; j < resource_num; ++ j)
				fprintf(stdout, "%d%c%c", allocation[i][j], (j == resource_num - 1) ? ']' : ',', (j == resource_num - 1) ? '\n' : ' ');
		}
	}
	
	//need
	if (printop >= 2) {
		fprintf(stdout, "  Need = \n");
		for (int i = 0; i < customer_num; ++ i) {
			fprintf(stdout, "    [");
			for (int j = 0; j < resource_num; ++ j)
				fprintf(stdout, "%d%c%c", need[i][j], (j == resource_num - 1) ? ']' : ',', (j == resource_num - 1) ? '\n' : ' ');
		}
	}
}



