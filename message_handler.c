#include "job_manager.h"
#include "die.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

char * message_handler(char * request, int n) {
    static int job_id = 1;

    char * cmd = strndup(request, n-1);
    Job *job = Job_create(job_id++, cmd, 99);

    if (JobManager_add_job(job) == 0)
        return "QUEUE FULL\n";

    if (Job_launch(job)) {
           return "OK\n";
    } else {
        return "FAIL\n";
    }
}
