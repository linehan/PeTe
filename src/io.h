#ifndef _P_IO_H_
#define _P_IO_H_ 
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>

#define DIR_PERMS ((S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH))

FILE *fopenf(const char *mode, const char *fmt, ...);
int   fmkdir(int perms, const char *fmt, ...);

#endif
