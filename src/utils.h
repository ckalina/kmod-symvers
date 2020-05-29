#ifndef __UTILS_H
#define __UTILS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void __attribute__ ((noreturn)) fail(const char *msg, ...)
{
	va_list ap;
	va_start(ap,msg);

	fprintf(stderr,"Error: ");
	vfprintf(stderr,msg,ap);
	fprintf(stderr,"\n");
	fflush(stderr);

	va_end(ap);
	exit(1);
}

const union {
    long one;
    char little;
} is_endian = { 1 };

#endif
