#ifndef _TDBG_H
#define _TDBG_H

#ifdef __unix__
#include <sys/types.h>
#else
//uh this isen't gonna end well
typedef int pid_t;
#endif

typedef struct tdbg_state {
	pid_t tracee;
	char **exe_args;
	int sig_handle;
} tdbg_state;

typedef struct tdbg_cmd {
	void (*func)(tdbg_state*,int,char**);
	char *names[8];
} tdbg_cmd;

void cmd(tdbg_state *ctx,char *buf);

#endif
