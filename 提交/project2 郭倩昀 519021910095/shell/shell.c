# include <stdio.h>
# include <fcntl.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>
# include <sys/types.h>
#define MAX_LINE	80 /* 80 chars per line, per command */
#define READ_END	0  // for pipe read
#define WRITE_END	1  // for pipe write

void reorganize(char *inst);		//reorganize the instrction to a standard form
int parse(char *inst,char **args);	//parse the instruction to args
void clearstr(char *str);			//clear the string



int main(void)
{
	char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
	int should_run = 1;
    
	char *inst, *last_inst;
    int concurrent=0;			//whether concurrent
    int have_last_inst=0;		//whether have last inst
    char *in_file, *out_file;	//redirect filename
    
    inst=(char*) malloc(MAX_LINE * sizeof(char));		//instruction
    last_inst=(char*) malloc(MAX_LINE * sizeof(char));	//for history
    in_file=(char*) malloc(MAX_LINE * sizeof(char));	//redirect filename
    out_file=(char*) malloc(MAX_LINE * sizeof(char));	//redirect filename
    //initialize
    clearstr(last_inst);
    clearstr(inst);
    
    pid_t pid;
			
    while (should_run){   
		printf("osh>");
		fflush(stdout);
		if(concurrent) wait(NULL);
		
		concurrent=0;
		clearstr(inst);
		
		fgets(inst,MAX_LINE,stdin);
		
		reorganize(inst);
		
		//check if concurrent
		if(strlen(inst)>0 && inst[strlen(inst)-1]=='&')
		{concurrent=1;}
		else concurrent=0;
		
		//exit
		if(strcmp(inst,"exit")==0)
		{
			should_run=0;
			continue;
		}
		//!! execute last inst
		if(strcmp(inst,"!!")==0)
		{
			if(have_last_inst==0)
			{
				fprintf(stderr,"  ERROR: No commands in history\n");
				continue;
			}
			else
			{
				printf("%s\n",last_inst);
				strcpy(inst, last_inst);
			}
		}
		//create child process
		pid = fork();
		if(pid<0){
			fprintf(stderr,"  ERROR: Fork Failed\n");
		}
		else
		{
			if(pid==0)//child
			{
				int error=0;
				//malloc args
				for(int i=0;i<MAX_LINE/2+1;i++)
				{
					args[i]=(char*)malloc(MAX_LINE*sizeof(char));
				}
				//parse to args
				int argn=parse(inst,args);
				
				for (int i = argn; i <= MAX_LINE / 2; ++ i) {
					free(args[i]);
					args[i] = NULL;
				}
				if (concurrent == 1) {
					-- argn;
					free(args[argn]);
					args[argn] = NULL;
				}
				
				//check | pipe
				int pipe_index=-1;
				for(int i=0;i<argn;i++)
				if(strcmp(args[i],"|")==0)
				{
					pipe_index=i;
					break;
				}
				if(pipe_index>=0)// found |
				{
					if(pipe_index==0||pipe_index>=argn-1)//
					{
						fprintf(stderr, "  ERROR: | illigal\n");
						error=1;
					}
				
					//pipe fd create
					int pipe_fd[2];
					if(pipe(pipe_fd)==-1)
					{
						fprintf(stderr,"  ERROR: Pipe failed\n");
						error=1;
					}
				
					if(error==0)
					{
						pid=fork();
					
						if(pid<0){
						fprintf(stderr,"  ERROR: Fork Failed\n");
						error=1;
						}
						else if(pid==0)//grandchild 
						{	//reorganize args
							for(int i=pipe_index;i<argn;i++)
							{
								free(args[i]);
								args[i]=NULL;
							}
							argn=pipe_index;
							close(pipe_fd[READ_END]);
							if(error==0&&dup2(pipe_fd[WRITE_END],STDOUT_FILENO)<0)
							{
								fprintf(stderr,"  ERROR: dup2 Failed\n");
								error=1;
							}
							if(error==0 && argn>0) 
								execvp(args[0],args);
							close(pipe_fd[WRITE_END]);
						
							for(int i=0;i<argn;++i) free(args[i]);
							free(inst);
							free(last_inst);
							free(in_file);
							free(out_file);
						
							exit(error);
						
						}
						else//child
						{
							wait(NULL);
							//reorganize args
							for(int i=0;i<=pipe_index;i++) free(args[i]);
							for(int i=pipe_index+1;i<argn;++i) args[i-pipe_index-1]=args[i];
							for(int i=argn-pipe_index-1;i<argn;i++) args[i]=NULL;
							argn=argn-pipe_index-1;

							close(pipe_fd[WRITE_END]);
							if(error==0&&dup2(pipe_fd[READ_END],STDIN_FILENO)<0)
							{
								fprintf(stderr,"  ERROR: dup2 Failed\n");
								error=1;
							}
							if(error==0 && argn>0) 
								execvp(args[0],args);
							close(pipe_fd[READ_END]);
						}
					
					}
				
				}
				else// | not found
				{
					int in_redirect=0;
					int in_fd=-1;
					int out_redirect=0;
					int out_fd=-1;
					//check < and reorganize args
					if(argn>2 && strcmp(args[argn-2],"<")==0)
					{
						in_redirect=1;
						strcpy(in_file,args[argn-1]);
						argn-=2;
						free(args[argn]);
						args[argn]=NULL;
						free(args[argn+1]);
						args[argn+1]=NULL;
					}
					//check > and reorganize args
					if(argn>2 && strcmp(args[argn-2],">")==0)
					{
						out_redirect=1;
						strcpy(out_file,args[argn-1]);
						argn-=2;
						free(args[argn]);
						args[argn]=NULL;
						free(args[argn+1]);
						args[argn+1]=NULL;
					}
					//in_redirect file open and redirect
					if(error==0&&in_redirect==1)
					{
						in_fd=open(in_file,O_RDONLY,0644);
						if(error==0 && in_fd<0)
						{
							fprintf(stderr,"  ERROR: no file\n");
							error=1;
						}
						if(error==0 && dup2(in_fd,STDIN_FILENO)<0)
						{
							fprintf(stderr,"  ERROR: dup2 Failed\n");
							error=1;
						}
					}
					//out_redirect file open and redirect
					if(error==0&&out_redirect==1)
					{
						out_fd=open(out_file,O_WRONLY|O_TRUNC|O_CREAT,0644);

						if(error==0 && dup2(out_fd,STDOUT_FILENO)<0)
						{
							fprintf(stderr,"  ERROR: dup2 Failed\n");
							error=1;
						}
					}
					//execute
					if(error==0 && argn>0)
						execvp(args[0],args);
					
					if(in_redirect==1 && in_fd>0) close(in_fd);
					if(out_redirect==1 && out_fd>0) close(out_fd);
				}
				
				if(error==0 && argn>0)
					execvp(args[0],args);
				//free
				for(int i=0;i<argn;++i) free(args[i]);
				free(inst);
				free(last_inst);
				free(in_file);
				free(out_file);
							
				exit(error);
			}
			else//parent
			{
				if(concurrent==0) wait(NULL);
			}
		}
		//for history
		if(have_last_inst==0) have_last_inst=1;
		strcpy(last_inst,inst); 
    }
    //free
    free(inst);
	free(last_inst);
	free(in_file);
	free(out_file);
    
	return 0;
}

//reorganize the instrction to a standard form
void reorganize(char *inst)
{
	int len1=strlen(inst);
	char *tmp=(char*) malloc (len1*sizeof(char));
	for(int i=0;i<len1;i++)
		tmp[i]=inst[i];
	clearstr(inst);
	
	int len2=0;
	int last_blank=1;
	for(int i=0;i<len1;++i)
	{
		if (tmp[i]==' '||tmp[i]=='\n'||tmp[i]=='\t')
		{	if(last_blank==0)
			{
				inst[len2]=' ';
				len2++;
				last_blank=1;
			}
		}
		else
		{
			inst[len2]=tmp[i];
			len2++;
			last_blank=0;
		}
	}
	if(inst[len2-1]==' ') inst[len2-1]=0;
	free(tmp);
}

//parse the instruction to args
int parse(char *inst,char **args)
{
	int instlen=strlen(inst);
	int argn =0;
	for(int i=0;i<instlen;i++)
	{
		clearstr(args[argn]);
		int j=i;
		for(j=i;j<instlen && inst[j]!=' ';j++)
		{args[argn][j-i]=inst[j];}
		//special case for < > |
		if(j>i+1 && (args[argn][0]=='<'||args[argn][0]=='>'||args[argn][0]=='|'))
		{
			strcpy(args[argn+1],args[argn]+1);
			for(int k=1;k<j-i;++k) args[argn][k]=0;
			argn++;
		}
		
		i=j;
		argn++;
	}
		return argn;
}

void clearstr(char *str)
{
	memset(str, 0, sizeof(str));
}

