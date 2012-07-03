#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "die.h"
#include "job.h"

Job *Job_create(int id, char *command, int time_limit) {
    assert(command != NULL);
    Job *job = malloc(sizeof(Job));
    assert(job != NULL);
    job->command = strdup(command);
    job->id = id;
    job->time_limit = time_limit;
    job->pid = -1;
    job->stdout_fd = -1;
    job->stderr_fd = -1;
    return job;
}

void Job_destroy(Job *job) {
    assert(job != NULL);
    free(job->command);
    if(job->stdout_fd > -1)
        close(job->stdout_fd);
    if(job->stderr_fd > -1)
        close(job->stderr_fd);
    free(job);
}

void Job_print(Job *job) {
    assert(job != NULL);
    printf("\nJob %d: %s time limit: %d.\n", job->id,
            job->command, job->time_limit);
}

int Job_launch(Job *job) {
    assert (job != NULL);

    // fd juggling adapted from here: http://jineshkj.wordpress.com/2006/12/22/how-to-capture-stdin-stdout-and-stderr-of-child-program/

    int child_stdout_fd[2], child_stderr_fd[2];

    // parent fds
    if(pipe(child_stdout_fd) == -1)
        die("Couldn't setup pipe");
    if(pipe(child_stderr_fd) == -1)
        die("Couldn't setup pipe");

    int pid = fork();
    if (pid == 0) { // child
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        // set write end of pipes as stdout & stderr
        if(dup2(child_stdout_fd[1], STDOUT_FILENO) == -1)
            die("Couldn't copy fd");
        if(dup2(child_stderr_fd[1], STDERR_FILENO) == -1)
            die("Couldn't copy fd");

        close(child_stdout_fd[0]);
        close(child_stderr_fd[0]);
        close(child_stdout_fd[1]);
        close(child_stderr_fd[1]);

        printf("Child executing %s\n", job->command);
        exit(execv(job->command, 0));
    }
    else if (pid > 0) { // parent
        job->pid = pid;
        // used by child
        close(child_stdout_fd[1]);
        close(child_stderr_fd[1]);
        // store fds
        job->stdout_fd = child_stdout_fd[0];
        job->stderr_fd = child_stderr_fd[0];
        printf("Job %d launched with pid %d\n", job->id, job->pid);
    }
    else {
        perror("Unable to fork!!");
        return 0;
    }

    return 1;
}
