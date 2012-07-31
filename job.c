#include <event2/event.h>
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

    // parse command arguments
    job->command = strdup(command);
    job->path = strtok(command, " ");
    char *p = job->path;
    job->argc = 0;
    while (p != NULL) {
        job->argv[job->argc] = p;
        job->argc++;
        p = strtok(NULL, " ");
    }

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
    if(job->stderr_ev != NULL)
        event_free(job->stderr_ev);
    if(job->stdout_ev != NULL)
        event_free(job->stdout_ev);
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

    // child output piping adapted from here: http://tinyurl.com/yzl5qvu

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

        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

        close(child_stdout_fd[0]);
        close(child_stderr_fd[0]);
        close(child_stdout_fd[1]);
        close(child_stderr_fd[1]);

        printf("Child executing '%s'\n", job->command);
        int r = execv(job->path, job->argv);
        if (r == -1)
            perror("Executing command");
        else
            printf("Child finished with code %d\n", r);
        exit(r);
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

static void Job_read_pipe(int fd, short flags, void *data) {
    assert(data != NULL);
    Job * job = data;
    char buf[1024];
    int ret = read(fd, buf, 1024);
    if (ret == -1)
        die ("Couldn't read pipe %d", fd);
    else if (ret > 0) {
        buf[ret] = 0;
        printf("%d > %s", job->id, buf);
    }
}

int Job_setup_pipes(Job *job, struct event_base *base) {
    assert(job != NULL);
    assert(base != NULL);
    job->stderr_ev = event_new(base, job->stderr_fd,
            EV_READ|EV_PERSIST, Job_read_pipe, (void*) job);

    assert(job->stderr_ev != NULL);
    event_add(job->stderr_ev, NULL);

    job->stdout_ev = event_new(base, job->stdout_fd,
            EV_READ|EV_PERSIST, Job_read_pipe, (void*) job);

    assert(job->stdout_ev != NULL);
    event_add(job->stdout_ev, NULL);

    return 1;
}

