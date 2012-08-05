#include "job_manager.h"
#include "event_server.h"
#include "die.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

char * message_handler(char * request, int n) {
    static int job_id = 1;

    char * cmd = strndup(request, n);
    Job *job = Job_create(job_id++, cmd, 99);
    if (job == NULL)
        return "FAILED TO CREATE JOB\n";

    if (JobManager_add_job(job) == 0)
        return "QUEUE FULL\n";

    if (Job_launch(job) && Job_setup_pipes(job))
        return "OK\n";
    else
        return "FAIL\n";
}
