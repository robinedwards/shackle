#include "job_manager.h"
#include "die.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>

JobManager * manager;

static void JobManager_child_exit_handler(int sig) {
    pid_t p;
    int status;

    printf("recievied signal %d\n", sig);

    while ((p=waitpid(-1, &status, WNOHANG)) != -1) {
        printf("pid %d just exited with status %d\n", p, status);
    }
}

void JobManager_create(int size) {
    if (manager != NULL)
        die("JobManager instance exists!");

    manager = malloc(sizeof(JobManager));
    assert(manager != NULL);

    manager->queue = malloc(size * sizeof(Job *));
    assert(manager->queue != NULL);

    manager->queue_size = size;

    // setup child handlers
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = JobManager_child_exit_handler;
    sigaction(SIGCHLD, &sa, NULL);
}

void JobManager_destroy() {
    assert(manager != NULL);

    for (int i = 0; i < manager->queue_size; i++)
        Job_destroy(manager->queue[i]);

    free(manager->queue);
    free(manager);
}

int JobManager_add_job(Job *job) {
    Job **slot = NULL;

    int i = 0;

    for (; i < manager->queue_size; i++) {
        if (manager->queue[i] == NULL)
            slot = &(manager->queue[i]);
    }

    if (slot == NULL) {
        printf("No free slots for job %d", job->id);
        return 0;
    }

    *slot = job;
    printf("Assigned job %d to slot %d\n", job->id, i);

    return 1;
}
