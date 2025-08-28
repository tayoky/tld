#ifndef _TTC_H
#define _TTC_H

extern const char *progname;

void error(const char *fmt,...);

// cute custom perror
#undef perror
#define perror(str) error("%s : %s",str,strerror(errno))

#endif
