#ifndef _TTC_H
#define _TTC_H

#include <stdint.h>
#ifdef __unix__
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#else
typedef long pid_t;
#endif

struct user_regs_struct;

#define ARCH_I386     1
#define ARCH_X86_64   2
#define ARCH_AARCH64  3

extern const char *progname;

void error(const char *fmt,...);
uintptr_t get_sp(pid_t tracee);
uintptr_t get_pc(pid_t tracee);
int get_regs(pid_t tracee, struct user_regs_struct *regs);
int ptrace_read(pid_t tracee,void *dest,uintptr_t src,size_t count);
void ptrace_show_backtrace(pid_t tracee,long max_depth);

// cute custom perror
#undef perror
#ifdef __unix__
#define perror(str) error("%s : %s",str,strerror(errno))
#else
#define perror(str) error("%s : unknow error",str)
#endif

#define arraylen(array) (sizeof(array)/sizeof(*array))

#endif
