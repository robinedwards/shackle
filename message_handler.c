#include "job.h"
#include "die.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char * message_handler(char * request, int n) {
    static int job_id = 1;
    int fd;

    char * cmd = strndup(request, n-1);
    Job *j = Job_create(job_id++, cmd, 99);
    Job_print(j);

    if((fd =  Job_launch(j)))
        return "OK";
    else
        return "FAIL";
}