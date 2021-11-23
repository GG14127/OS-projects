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

void clearstr(char *str)
{
	memset(str, 0, sizeof(str));
}

int isconcurrent(char *inst)
{
	int len=strlen(inst);
	if(len>0 && inst[len-1]=='&') 
		return 1;
	return 0;
}

void reorganize(char *inst)
{
	int len1=strlen(inst);
	char *tmp=(char*) malloc (len1*sizeof(char));
	for(int i=0;i<len1;i++)//
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
		
		if(j>i+1 && (args[argn][0]=='<'||args[argn][0]=='>'||args[argn][0]=='|'))//< > |
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

void printargs(char *args[],int argn)
{
	fprintf(stderr,"total %d args\n", argn);
	for(int i=0;i<argn;i++)
	{
		fprintf(stderr,"args[%d]=%s", i,args[i]);
	}
}

int main(void)
{
	char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
    int should_run = 1;
    
    char *inst, *last_inst;
    int concurrent=0;//whether concurrent
    int have_last_inst=0;//whether have last inst
    char *in_file, *out_file;//redirect filename
    
    inst=(char*) malloc(MAX_LINE * sizeof(char));
    last_inst=(char*) malloc(MAX_LINE * sizeof(char));
    in_file=(char*) malloc(MAX_LINE * sizeof(char));
    out_file=(char*) malloc(MAX_LINE * sizeof(char));
    
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
		concurrent=isconcurrent(inst);
		
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
				fprintf(stderr,"No commands in history\n");
				continue;
			}
			else
			{
				printf("%s\n",last_inst);
				strcpy(inst, last_inst);
			}
		}
		
		pid = fork();
		if(pid<0){
			fprintf(stderr,"Fork Failed\n");
		}
		else
		{
			if(pid==0)//child
			{
				int error=0;
				//space
				for(int i=0;i<MAX_LINE/2+1;i++)
				{
					args[i]=(char*)malloc(MAX_LINE*sizeof(char));
				}
			
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
			
				//printargs(args,argn);
				
				
				//find | pipe
				int pipe_index=-1;
				for(int i=0;i<argn;i++)
				if(strcmp(args[i],"|")==0)
				{
					pipe_index=i;
					break;
				}
				//printf("pipe_index=%d\n",pipe_index);
				if(pipe_index>=0)
				{
					if(pipe_index==0||pipe_index>=argn-1)//
					{
						fprintf(stderr, "ERROR: | illigal\n");
						error=1;
					}
				
					//pipe fd create
					int pipe_fd[2];
					if(pipe(pipe_fd)==-1)
					{
						fprintf(stderr,"Pipe failed\n");
						error=1;
					}
				
					if(error==0)
					{
						pid=fork();
					
						if(pid<0){
						fprintf(stderr,"Fork Failed\n");
						error=1;
						}
						else if(pid==0)//grandchild
						{
							for(int i=pipe_index;i<argn;i++)
							{
								free(args[i]);
								args[i]=NULL;
							}
							argn=pipe_index;

							//printargs(args,argn);

							close(pipe_fd[READ_END]);
							if(error==0&&dup2(pipe_fd[WRITE_END],STDOUT_FILENO)<0)
							{
								fprintf(stderr,"ERROR:dup2 Failed\n");
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
						else
						{
							wait(NULL);
							for(int i=0;i<=pipe_index;i++) free(args[i]);
							for(int i=pipe_index+1;i<argn;++i) args[i-pipe_index-1]=args[i];
							for(int i=argn-pipe_index-1;i<argn;i++) args[i]=NULL;
							argn=argn-pipe_index-1;

							//printargs(args,argn);

							close(pipe_fd[WRITE_END]);
							if(error==0&&dup2(pipe_fd[READ_END],STDIN_FILENO)<0)
							{
								fprintf(stderr,"ERROR:dup2 Failed\n");
								error=1;
							}
							if(error==0 && argn>0) 
								execvp(args[0],args);
							close(pipe_fd[READ_END]);
						}
					
					}
				
				}
				else
				{
					int in_redirect=0;
					int in_fd=-1;
					int out_redirect=0;
					int out_fd=-1;
				
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
				
					if(error==0&&in_redirect==1)
					{
						in_fd=open(in_file,O_RDONLY,0644);
						if(error==0 && in_fd<0)
						{
							fprintf(stderr,"ERROR:no file\n");
							error=1;
						}
						if(error==0 && dup2(in_fd,STDIN_FILENO)<0)
						{
							fprintf(stderr,"ERROR:dup2 Failed\n");
							error=1;
						}
					}
				
					if(error==0&&out_redirect==1)
					{
						out_fd=open(out_file,O_WRONLY|O_TRUNC|O_CREAT,0644);

						if(error==0 && dup2(out_fd,STDOUT_FILENO)<0)
						{
							fprintf(stderr,"ERROR:dup2 Failed\n");
							error=1;
						}
					}
					
					if(error==0 && argn>0)
						execvp(args[0],args);
					
					if(in_redirect==1 && in_fd>0) close(in_fd);
					if(out_redirect==1 && out_fd>0) close(out_fd);
				}
				
				if(error==0 && argn>0)
						execvp(args[0],args);
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
		if(have_last_inst==0) have_last_inst=1;
		strcpy(last_inst,inst); 

    }
    free(inst);
	free(last_inst);
	free(in_file);
	free(out_file);
    
	return 0;
}
//gcc shell.c -o shell
//ls -l /bin/sh
//sudo dpkg-reconfigure dash
