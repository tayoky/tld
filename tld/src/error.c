#include <stdio.h>
#include <stdarg.h>
#include "tld.h"

void error(const char *fmt,...){
	va_list args;
	va_start(args,fmt);
	fputs("tld : ",stderr);
	vfprintf(stderr,fmt,args);
	fputc('\n',stderr);
	va_end(args);
}
