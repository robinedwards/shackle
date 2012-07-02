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
    job->stdin_fd = -1;
    job->stdout_fd = -1;
    job->stderr_fd = -1;
    return job;
}

void Job_destroy(Job *job) {
    assert(job != NULL);
    free(job->command);
    if(job->stdin_fd > -1)
        close(job->stdin_fd);
    if(job->stdout_fd > -1)
        close(job->stdout_fd);
    if(job->stderr_fd > -1)
        close(job->stdin_fd);
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

    int child_stdout_fd[2];
    int child_stdin_fd[2];
    int child_stderr_fd[2];
    int old_stdin, old_stdout, old_stderr;

    // parent fds
    pipe(child_stdin_fd);
    pipe(child_stdout_fd);
    pipe(child_stderr_fd);

    // save current fds
    old_stdin = dup(STDIN_FILENO);
    old_stdout = dup(STDOUT_FILENO);
    old_stderr = dup(STDERR_FILENO);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // set read end of pipe as stdin
    dup2(child_stdin_fd[0], STDIN_FILENO);

    // set write end of pipes as stdout & stderr
    dup2(child_stdout_fd[1], STDOUT_FILENO);
    dup2(child_stderr_fd[1], STDERR_FILENO);

    int pid = fork();
    if (pid == 0) { // child
        close(child_stdin_fd[0]);
        close(child_stdout_fd[0]);
        close(child_stderr_fd[0]);
        close(child_stdin_fd[1]);
        close(child_stdout_fd[1]);
        close(child_stderr_fd[1]);

        printf("Child executing %s\n", job->command);
        execv(job->command, 0);
        printf("Child finished\n");
    }
    else if (pid > 0) { // parent
        job->pid = pid;
        // restore old fds
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        dup2(old_stdin, STDIN_FILENO);
        dup2(old_stdout, STDOUT_FILENO);
        dup2(old_stderr, STDERR_FILENO);
        // used by child
        close(child_stdin_fd[0]);
        close(child_stdout_fd[1]);
        close(child_stderr_fd[1]);
        // store fds
        job->stdin_fd = child_stdin_fd[1];
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
