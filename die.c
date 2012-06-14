#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

void die(const char *format, ...) {

    if(errno) {
        char *p = strerror(errno);
        printf("ERROR: %s\n", p);
    } else {
        va_list args;
        va_start (args, format);
        char buffer[1024];
        sprintf(buffer, format, args);
        va_end(args);
        printf("ERROR: %s\n", buffer);
    }

    exit(1);
}
