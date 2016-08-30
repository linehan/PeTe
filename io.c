#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "io.h"

/**
 * fopenf()
 * ````````
 * Open different kinds of files as a file descriptor.
 * 
 * @mode : Mode string. Similar to fopen()
 * @fmt  : Path to open, as a format string
 * @...  : Format string arguments
 * Return: FILE stream pointer, or -1 on error.
 */
FILE *fopenf(const char *mode, const char *fmt, ...) 
{
        char path[4096];

        va_list args;
        va_start(args, fmt); 
        vsnprintf(path, 4096, fmt, args);
        va_end(args);

        return fopen(path, mode);
}


/**
 * fmkdir()
 * ````````
 * Create a new directory with a format string. 
 *
 * @perms: Permissions for directory
 * @fmt  : Directory path, as a format string
 * @...  : Format string arguments
 * Return: nothing.
 */
int fmkdir(int perms, const char *fmt, ...)
{
        char path[4096];

        va_list args;
        va_start(args, fmt); 
        vsnprintf(path, 4096, fmt, args);
        va_end(args);

        return mkdir(path, perms);
}

