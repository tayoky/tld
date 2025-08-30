#include <sys/ptrace.h>
#include "libttc.h"

int ptrace_read(pid_t tracee,void *dest,uintptr_t src,size_t count){
	if(!count)return 0;
	uint8_t *buf = dest;
	uintptr_t addr = src / sizeof(int) * sizeof(int);
	uintptr_t end = src + count;
	while(addr < end){
		unsigned int data = (unsigned int)ptrace(PTRACE_PEEKDATA,tracee,addr,0);
		for(size_t i=0; i<sizeof(int); i++){
			//allow unaligned read
			if(addr == src && src % sizeof(int))continue;
			if(count-- > 0){
				*(buf++) = data;
				data >>= 8;
			}
		}
		addr += sizeof(int);
	}
	return 0;
}
