#include "job_manager.h"
#include "die.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>

JobManager * manager;

static int JobManager_remove_job(int pid) {
    for (int i = 0; i < manager->queue_size; i++) {
        if (manager->queue[i] == NULL) continue;

        if (manager->queue[i]->pid == pid) {
            printf("Found child %d in slot %d, job %d complete\n",
                    pid, i, manager->queue[i]->id);

            Job_destroy(manager->queue[i]);
            manager->queue[i] = NULL;
            return 1;
        }

    }

    return 0;
}

static void JobManager_child_exit_handler(int sig) {
    pid_t p;
    int status;

    while ((p=waitpid(-1, &status, WNOHANG)) > 0) {
        printf("pid %d just exited with status %d\n", p, status);
        if (!JobManager_remove_job(p)) {
            printf("Couldn't find corresponding job in the queue?\n");
        }
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

    for (; i <= manager->queue_size; i++) {
        if (manager->queue[i] == NULL) {
            slot = &(manager->queue[i]);
            printf("found free slot %d for job %d > %p\n", i, job->id, manager->queue);
            break;
        }
    }

    if (slot == NULL) {
        printf("No free slots for job %d\n", job->id);
        return 0;
    }

    *slot = job;

    return 1;
}
