#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "tdbg.h"
#include "libttc.h"

#define CMD(f,...) {.func = f,.names = { __VA_ARGS__ , NULL}}

void quit(tdbg_state *ctx,int argc,char **argv){
	(void)argc;
	(void)argv;
	if(ctx->tracee)kill(ctx->tracee,SIGKILL);
	exit(0);
}

void run(tdbg_state *ctx,int argc,char **argv){
	if(ctx->tracee){
		error("already tracing a process");
		return;
	}
	if(argc >= 2){
		if(ctx->exe_args){
			for(int i=0; ctx->exe_args[i]; i++){
				free(ctx->exe_args[i]);
			}
			free(ctx->exe_args);
		}
		ctx->exe_args = malloc(sizeof(char *) * argc);
		for(int i=0; i<argc-1; i++){
			ctx->exe_args[i] = strdup(argv[i+1]);
		}
		ctx->exe_args[argc-1] = NULL; strdup(argv[1]);
	} else if(!ctx->exe_args){
		error("missing argument");
		return;
	}
	fflush(stdout);
	pid_t child = fork();
	if(!child){
		//attatch to parent
		if(ptrace(PTRACE_TRACEME) < 0){
			perror("ptrace");
			exit(1);
		}
		//no need for a sig stop, execvp already trigger a SIGTRAP
		execvp(ctx->exe_args[0],ctx->exe_args);
		perror(ctx->exe_args[0]);
		exit(0);
	}
	if(child < 0){
		perror("fork");
		return;
	}

	//first wait until child is stopped
	int status;
	if(waitpid(child,&status,0) < 0){
		perror("waitpid");
		return;
	}
	if(!WIFSTOPPED(status)){
		error("child did not stop");
		kill(child,SIGKILL);
		return;
	}

	printf("trace %ld\n",(long)child);
	if(ptrace(PTRACE_CONT,child,0,0) < 0){
		perror("ptrace");
		kill(child,SIGKILL);
		return;
	}

	//now wait
	ctx->tracee = child;
	for(;;){
		if(waitpid(ctx->tracee,&status,0) < 0){
			perror("waitpid");
			return;
		}
		if(WIFEXITED(status)){
			printf("exit with status %d\n",WEXITSTATUS(status));
			return;
		}
		if(WIFSIGNALED(status)){
			printf("recive %s\n",strsignal(WEXITSTATUS(status)));
			ptrace(PTRACE_CONT,child,0,0);
		}
	}
}

tdbg_cmd commands[] = {
	CMD(quit,"quit","q","exit"),
	CMD(run ,"run"),
};

void cmd(tdbg_state *ctx,char *buf){
	//tokenise in arguments
	int argc = 0;
	char **argv = NULL;
	int prev_is_space = 1;
	while(*buf && *buf != '\n'){
		if(*buf == ' ' || *buf == '\t'){
			prev_is_space = 1;
			*buf = '\0';
		} else if(prev_is_space){
			prev_is_space = 0;
			argc++;
			argv = realloc(argv,sizeof(char *) * argc);
			argv[argc-1] = buf;
		}
		buf++;
	}
	*buf = '\0';
	argv = realloc(argv,sizeof(char *) * (argc+1));
	argv[argc] = NULL;

	if(argc == 0)return;
	for(size_t i=0; i<arraylen(commands); i++){
		for(size_t j=0; commands[i].names[j]; j++){
			if(!strcasecmp(argv[0],commands[i].names[j])){
				commands[i].func(ctx,argc,argv);
				free(argv);
				return;
			}
		}
	}
	free(argv);
	error("unknow command '%s'",argv[0]);
}
