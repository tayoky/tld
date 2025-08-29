#include <elf.h>
#ifdef __ethereal__
#include <ethereal/user.h>
#else
#include <sys/user.h>
#endif
#include <sys/uio.h>
#include <sys/ptrace.h>
#include "libttc.h"

#ifdef __x86_64__
#define SP rsp
#define BP rbp
#define PC rip
#elif defined(__i386__)
#define SP esp
#define BP ebp
#define PC eim
#elif defined(__aarch64__)
#define SP sp
#define PC pc
#else
//unsupported arch
#define UNSUPPORTED_ARCH 1
#endif

#ifdef UNSUPPORTED_ARCH
#define FUNC(name,reg) uintptr_t name(pid_t tracee){\
	(void)tracee;\
	error("unsupported arch");\
	return 0;\
}
#elif defined(PTRACE_GETREGS)
#define FUNC(name,reg) uintptr_t name(pid_t tracee){\
	struct user_regs_struct regs;\
	if(ptrace(PTRACE_GETREGS,tracee,&regs,0) < 0){\
		perror("ptrace");\
		return 0;\
	}\
	return regs.reg;\
}
#else
#define FUNC(name,reg) uintptr_t name(pid_t tracee){\
	struct user_regs_struct regs;\
	struct iovec vec = {\
		.iov_base = &regs,\
		.iov_len  = sizeof(regs),\
	};\
	if(ptrace(PTRACE_GETREGSET,tracee,(void*)NT_PRSTATUS,&vec,0) < 0){\
		perror("ptrace");\
		return 0;\
	}\
	return regs.reg;\
}
#endif

FUNC(get_sp,SP)
FUNC(get_pc,PC)
