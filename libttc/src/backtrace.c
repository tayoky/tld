#ifdef __ethereal__
#include <ethereal/user.h>
#else
#include <sys/user.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include "libttc.h"

static uintptr_t get_next_frame(pid_t tracee,struct user_regs_struct *regs){
	uintptr_t addr = 0;
#ifdef __i386__
	if(!regs->ebp)return addr;
	addr = regs->eip;
	uint32_t frame[2];
	if(ptrace_read(tracee,frame,regs->ebp,sizeof(frame)) < 0)return 0;
	regs->ebp = frame[0];
	regs->eip = frame[1];
#elif defined(__x86_64__)
	if(!regs->rbp)return addr;
	addr = regs->rip;
	uint64_t frame[2];
	if(ptrace_read(tracee,frame,regs->rbp,sizeof(frame)) < 0)return 0;
	regs->rbp = frame[0];
	regs->rip = frame[1];
#elif defined(__aarch64__)
	if(!regs->regs[29])return addr;
	addr = regs->pc;
	uint64_t frame[2];
	if(ptrace_read(tracee,frame,regs->regs[29],sizeof(frame)) < 0)return 0;
	regs->regs[29] = frame[0];
	regs->pc       = frame[1];
#else
	(void)tracee;
#endif
	return addr;
}

void ptrace_show_backtrace(pid_t tracee,long max_depth){
	struct user_regs_struct regs;
	if(get_regs(tracee,&regs) < 0){
		error("backtrace unavailable");
		return;
	}
	int deep = 0;
	uintptr_t addr;
	while(max_depth-- > 0 && (addr = get_next_frame(tracee,&regs)) > 0){
		printf("#%02d 0x%p\n",deep++,(void *)addr);
	}
}
