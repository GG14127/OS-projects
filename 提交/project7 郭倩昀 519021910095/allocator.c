# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define MAX_LINE 500
# define TRUE 1

int memory;
typedef struct mem_node{
	char *process;
	int start;
	int end;
	struct mem_node *next;
}mem_node;
struct mem_node *head=NULL;
//request for a contiguous block of memory
int request(char *process, int size, char strategy);
//release of a contiguous block of memory
int release(char *process);
//compact unused holes of memory into one single block
void compact();
//report the regions of free and allocated memory
void report();

int main(int argc, char *argv[])
{
	if (argc != 2) 
	{
		fprintf(stderr, "  ERROR: Arguments Error\n");
		exit(1);
	}
	//get memory size
	memory=atoi(argv[1]);
	char arg[MAX_LINE];		//full instrucion
	char op[MAX_LINE];		//operation
	char process[MAX_LINE];	//process name
	while(TRUE)
	{
		for (int i = 0; i < MAX_LINE; ++ i) //initialize
		{
			arg[i] = 0;
			op[i] = 0;
			process[i] = 0;
		}
		fprintf(stdout, "allocator> ");
		fgets(arg, MAX_LINE, stdin);
		
		//standardize arg
		char tmp[MAX_LINE];
		int last_blank = 1;
		int dex = 0;
		for (int i = 0; arg[i]; ++ i) {
			if (arg[i] == ' ' || arg[i] == '\t' || arg[i] == '\n') {
				if (last_blank == 0) {
					last_blank = 1;
					tmp[dex ++] = ' ';
				}
			} else {
				tmp[dex++] = arg[i];
				last_blank = 0;
			}
		}
		if (dex > 0 && tmp[dex - 1] == ' ') dex --;
		for (int i = 0; i < dex; ++ i) arg[i] = tmp[i];
		arg[dex] = 0;
		
		if (strcmp(arg, "X") == 0)	//EXIT
			break;	
		if (strcmp(arg, "C") == 0)	//COMPACT
		{
			compact();
			continue;
		}
		if (strcmp(arg, "STAT") == 0)//REPORT
		{
			report();		
			continue;
		}
		
		for(dex=0;arg[dex];dex++)
		{
			if(arg[dex]==' ')
				break;
		}
		
		//get op
		for (int i = 0; i < dex; ++ i)
		{	op[i] = arg[i];}
		op[dex] = 0;
		
		//op
		if (strcmp(op, "RQ") == 0) //new process request
		{
			if (arg[dex] == 0) 
			{
				fprintf(stderr, "  ERROR: Invalid input\n");
				continue;
			}
			
			int i;
			int invalid= 0;
			int size = 0;
			char strategy;
			//get process name
			for (i = dex + 1; arg[i]; i++) 
			{
				if (arg[i] == ' ') break;
				process[i-dex-1] = arg[i];
			}
			process[i-dex-1]=0;
			if (arg[i] == 0) 
			{
				fprintf(stderr, "  ERROR: Invalid input\n");
				continue;
			}
			
			//get size and check
			dex = i;
			for (i = dex + 1; arg[i]; ++ i) 
			{
				if(arg[i] == ' ') break;
				if(arg[i] < '0' || arg[i] > '9') 
				{
					invalid = 1;
					break;
				}
				size = size * 10 + arg[i] - '0';
			}
			if (invalid || arg[i] == 0) 
			{
				fprintf(stderr, "  ERROR: Invalid input\n");
				continue;
			}
			if (size<=0) 
			{
				fprintf(stderr, "  ERROR: Size invalid\n");
				continue;
			}
			
			//get strategy and check
			dex = i;
			if(arg[dex+1]==0||arg[dex+2]!=0)
			{
				fprintf(stderr, "  ERROR: Invalid input\n");
				continue;
			}
			strategy=arg[dex+1];
			request(process, size, strategy);
		}
		else if (strcmp(op, "RL") == 0)
		{
			if (arg[dex] == 0) {
				fprintf(stderr, "  ERROR: Invalid input\n");
				continue;
			}
			int invalid= 0;
			//get process name
			int i;
			for (i = dex + 1; arg[i]; i++) {
				if (arg[i] == ' ')
				{ invalid=1;break;}
				process[i-dex-1] = arg[i];
			}
			process[i-dex-1]=0;
			if (invalid) {
				fprintf(stderr, "  ERROR: Invalid input\n");
				continue;
			}
			release(process);
		}
		else
		{
			fprintf(stderr, "  ERROR: Invalid input\n");
			continue;
		}
	}
	return 0;
}

//request for a contiguous block of memory
int request(char *process, int size, char strategy)
{
	int name_len=strlen(process);
	int hole_len;
	if (head == NULL) {
		if (size <= memory) {	
			head = (mem_node *) malloc (sizeof(mem_node));
			head -> process = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(head -> process, process);
			head -> start = 0; 
			head -> end = 0 + size - 1;
			head -> next = NULL;
			return 0;
		} else {
			fprintf(stderr, "  ERROR: No enough space\n");
			return 1;
		}	
	}
	
//first fit
	if(strategy=='F')
	{
		mem_node *p = head;
		
		//first hole
		hole_len = p -> start - 0;
		if (size <= hole_len)	//fit in
		{
			mem_node *tmp = head;
			head = (mem_node *) malloc (sizeof(mem_node));
			head -> process = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(head -> process, process);
			head -> start = 0; 
			head -> end = 0 + size - 1;
			head -> next = tmp;
			return 0;
		}
		
		//middle
		while (p -> next != NULL)	//search
		{
			hole_len = p->next->start - p->end - 1;
			if (size <= hole_len)	//fit in
			{
				mem_node *tmp = p->next;
				p->next = (mem_node *) malloc (sizeof(mem_node));
				p->next->process = (char *) malloc (sizeof(char) * (name_len + 1));
				strcpy(p->next->process, process);
				p->next->start = p->end + 1;
				p->next->end = p->next->start + size - 1;
				p->next->next = tmp;
				return 0;
			}
			p = p -> next;
		}
		
		//last hole
		hole_len = memory - p->end - 1;
		if (size <= hole_len) 
		{
			p->next = (mem_node *) malloc (sizeof(mem_node));
			p->next->process = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(p->next->process, process);
			p->next->start = p->end + 1;
			p->next->end = p->next->start + size - 1;
			p->next->next = NULL;
			return 0;
		}

		// No enough space
		fprintf(stderr, "  ERROR: No enough space\n");
		return 1;
	}
	
//best fit
	if (strategy == 'B') 
	{
		mem_node *p = head;
		int min_best = memory;		
		int type = 0;
		mem_node *best;
		
		//search hole
		//first hole
		hole_len = p -> start - 0;
		if (size <= hole_len && hole_len < min_best) {
			min_best = hole_len;
			type = 1;
		}
		//middle
		while (p -> next != NULL) {
			hole_len = p -> next -> start - p -> end - 1;
			if (size <= hole_len && hole_len < min_best) {
				min_best = hole_len;
				type = 2;
				best = p;
			}
			p = p -> next;
		}
		//last hole
		hole_len = memory - p -> end - 1;
		if (size <= hole_len && hole_len < min_best) {
			min_best = hole_len;
			type = 3;
		}
		// No enough space
		if (type == 0) {
			fprintf(stderr, "[Err] No enough spaces!\n");
			return 1;
		}
		
		//allocate memory
		//first hole
		if (type == 1) 
		{
			mem_node *tmp = head;
			head = (mem_node *) malloc (sizeof(mem_node));
			head -> process = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(head -> process, process);
			head -> start = 0; 
			head -> end = 0 + size - 1;
			head -> next = tmp;
			return 0;
		}
		//middle
		if (type == 2) 
		{
			p = best;
			mem_node *tmp = p->next;
			p->next = (mem_node *) malloc (sizeof(mem_node));
			p->next->process = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(p->next->process, process);
			p->next->start = p->end + 1;
			p->next->end = p->next->start + size - 1;
			p->next->next = tmp;
			return 0;
		}
		//last hole
		if (type == 3) 
		{
			p->next = (mem_node *) malloc (sizeof(mem_node));
			p->next->process = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(p->next->process, process);
			p->next->start = p->end + 1;
			p->next->end = p->next->start + size - 1;
			p->next->next = NULL;
			return 0;
		}
	}
	
//worst fit
	if (strategy== 'W') 
	{
		mem_node *p = head;
		int max_worst = 0;		
		int type = 0;
		mem_node *worst;
		
		//search hole
		//first hole
		hole_len = p -> start - 0;
		if (size <= hole_len && hole_len > max_worst) 
		{
			max_worst = hole_len;
			type = 1;
		}
		//middle
		while (p -> next != NULL) 
		{
			hole_len = p -> next -> start - p -> end - 1;
			if (size <= hole_len && hole_len > max_worst) {
				max_worst = hole_len;
				type = 2;
				worst = p;
			}
			p = p -> next;
		}
		//last hole
		hole_len = memory - p -> end - 1;
		if (size <= hole_len && hole_len > max_worst) 
		{
			max_worst = hole_len;
			type = 3;
		}
		// No enough space
		if (type == 0) {
			fprintf(stderr, "[Err] No enough spaces!\n");
			return 1;
		}
		
		//allocate memory
		//first hole
		if (type == 1) 
		{
			mem_node *tmp = head;
			head = (mem_node *) malloc (sizeof(mem_node));
			head -> process = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(head -> process, process);
			head -> start = 0; 
			head -> end = 0 + size - 1;
			head -> next = tmp;
			return 0;
		}
		//middle
		if (type == 2) 
		{
			p = worst;
			mem_node *tmp = p->next;
			p->next = (mem_node *) malloc (sizeof(mem_node));
			p->next->process = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(p->next->process, process);
			p->next->start = p->end + 1;
			p->next->end = p->next->start + size - 1;
			p->next->next = tmp;
			return 0;
		}
		//last hole
		if (type == 3) 
		{
			p->next = (mem_node *) malloc (sizeof(mem_node));
			p->next->process = (char *) malloc (sizeof(char) * (name_len + 1));
			strcpy(p->next->process, process);
			p->next->start = p->end + 1;
			p->next->end = p->next->start + size - 1;
			p->next->next = NULL;
			return 0;
		}
	}
//error argument
	fprintf(stderr, "  ERROR: Arguments Error\n");
	return 1;
	
}

//release of a contiguous block of memory
int release(char *process)
{
	mem_node *p = head;
	if (head == NULL) {
		fprintf(stderr, "  ERROR: No such process\n");
		return 1;
	}
	//first
	if (strcmp(head -> process, process) == 0) {
		mem_node *tmp = head;
		head = head -> next;
		free(tmp -> process);
		free(tmp);
		return 0;
	}
	//search
	while (p -> next != NULL) 
	{
		if (strcmp(p->next->process, process) == 0) 
		{
			mem_node *tmp = p -> next;
			p -> next = p -> next -> next;
			free(tmp -> process);
			free(tmp);
			return 0;
		}
		p = p -> next;
	}
	//not found
	fprintf(stderr, "  ERROR: No such process\n");
	return 1;
}

//compact unused holes of memory into one single block
void compact()
{
	int pos = 0;
	mem_node *p = head;
	while (p != NULL) {
		int size = p->end - p->start + 1;
		p->start = pos;
		p->end = pos + size - 1;
		pos += size;		
		p = p->next;
	}
	return;
}
//report the regions of free and allocated memory
void report()
{
	mem_node *p = head;
	//first
	if (head == NULL) 
	{
		fprintf(stdout, "  Address [0 : %d] Unused\n", memory - 1);
		return ;
	} 
	else if (head -> start != 0) 
	{
		fprintf(stdout, "  Address [0 : %d] Unused\n", head -> start - 1);
	}
	//middle
	while (p -> next != NULL) {
		fprintf(stdout, "  Address [%d : %d] Process %s\n", p->start, p->end, p->process);
		if (p->next->start - p->end - 1 > 0) 
		{
			fprintf(stdout, "  Address [%d : %d] Unused\n", p->end + 1, p->next->start - 1);
		}
		p = p->next;
	}
	
	fprintf(stdout, "  Address [%d : %d] Process %s\n", p->start, p->end, p->process);
	if (memory - p->end - 1 > 0)
		fprintf(stdout, "  Address [%d : %d] Unused\n", p->end + 1, memory - 1);

}
