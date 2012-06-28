#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "die.h"
#include "job.h"

Job *Job_create(int id, char *command, int time_limit) {
    assert(command != NULL);
    Job *job = malloc(sizeof(Job));
    assert(job != NULL);
    job->command = strdup(command);
    job->id = id;
    job->time_limit = time_limit;
    return job;
}

void Job_destroy(Job *job) {
    assert(job != NULL);
    free(job->command);
    free(job);
}

void Job_print(Job *job) {
    assert(job != NULL);
    printf("\nJob %d: %s time limit: %d.\n", job->id,
            job->command, job->time_limit);
}

int Job_launch(Job *job) {
    assert (job != NULL);
    FILE *pipe;

    if ((pipe = popen(job->command, "r"))) {
        job->fd = fileno(pipe);
        fcntl(job->fd, F_SETFL, O_NONBLOCK);
        return 1;
    }
    else {
        die("Couldn't execute: %s", 2, job->command);
    }

    return 0;
}
