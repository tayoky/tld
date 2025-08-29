#ifndef _TTC_H
#define _TTC_H

#ifdef __unix__
#include <string.h>
#include <errno.h>
#endif

extern const char *progname;

void error(const char *fmt,...);

// cute custom perror
#undef perror
#ifdef __unix__
#define perror(str) error("%s : %s",str,strerror(errno))
#else
#define perror(str) error("%s : unknow error",str)
#endif

#define arraylen(array) (sizeof(array)/sizeof(*array))

#endif
