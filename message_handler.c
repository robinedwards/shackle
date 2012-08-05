#include "job_manager.h"
#include "event_server.h"
#include "config.h"
#include "die.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int message_handler(char * request, char *response, int n) {
    static int job_id = 1;

    char * cmd = strndup(request, n);
    Job *job = Job_create(job_id++, cmd, 99);
    if (job == NULL)
        return sprintf(response, "MEGAFAIL\n");

    if (JobManager_add_job(job) == 0)
        return sprintf(response, "FULL\n");

    if (Job_launch(job) && Job_setup_pipes(job)) {
        return sprintf(response, "OK %d:%d\n", config->node_id, job->id);
    } else
        return sprintf(response, "FAIL\n");
}
