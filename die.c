#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

void die(const char *format, ...) {
    va_list args;
    va_start (args, format);
    char buffer[1024];
    sprintf(buffer, format, args);
    va_end(args);
    printf("ERROR: %s\n", buffer);
    if(errno) {
        char *p = strerror(errno);
        printf("PERROR: %s\n", p);
    }
    exit(1);
}
