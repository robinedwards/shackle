#include <event2/event.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <json/json.h>
#include "die.h"
#include "job.h"
#include "config.h"

extern struct event_base *event_base;

Job *Job_create(int id, char *command, int time_limit) {
    assert(command != NULL);
    Job *job = malloc(sizeof(Job));
    assert(job != NULL);
    job->stderr_output = NULL;
    job->stdout_output = NULL;

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
    job->argv[job->argc] = NULL;

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
    if(job->stdout_output != NULL)
        free(job->stdout_output);
    if(job->stderr_output != NULL)
        free(job->stderr_output);
    free(job);
}

void Job_print(Job *job) {
    assert(job != NULL);
    printf("\nJob %d: %s time limit: %d.\n", job->id,
            job->command, job->time_limit);

    if(job->stdout_output != NULL)
        puts(job->stdout_output);

    if(job->stderr_output != NULL)
        puts(job->stderr_output);
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

        // set write end of pipes as stdout & stderr
        if(dup2(child_stdout_fd[1], STDOUT_FILENO) == -1)
            die("Couldn't copy fd");
        if(dup2(child_stderr_fd[1], STDERR_FILENO) == -1)
            die("Couldn't copy fd");

        // close all fd appart from stdout + stderr
        for (int i = getdtablesize(); i >= 3; --i) close(i);
        close(STDIN_FILENO);

        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);

        printf("Child executing '%s'\n", job->command);
        int r = execv(job->path, job->argv);
        if (r == -1)
            die("Executing command");
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
        return 1;
    }
    else {
        perror("Unable to fork!!");
        return 0;
    }
    return 1;
}

static void Job_grow_and_append(char **p, char *data, int size) {
    data[size] = 0;
    // alloc or expand
    if (*p == NULL) {
        *p = malloc(sizeof(char) * 1024 * 1024);
        **p = 0;
    } /* TODO else {
        printf("old %lu new %lu\n", sizeof(*p), sizeof(*p) + sizeof(char) * size);
        *p = realloc(*p,  sizeof(*p) + sizeof(char) * size);
    } */

    *p = strcat(*p, data);
}

static void Job_read_stdout_pipe(int fd, short flags, void *data) {
    assert(data != NULL);
    Job * job = data;

    char buf[1024];
    int ret = read(fd, buf, 1024);
    if (ret == -1) {
        close(fd);
       // perror("Couldn't read stdout pipe");
    }
    else if (ret > 0) {
        buf[ret] = 0;
        Job_grow_and_append(&(job->stdout_output), buf, ret);
        struct event * stdout_ev = event_new(event_base, fd, EV_READ, Job_read_stdout_pipe, (void*) job);
        event_add(stdout_ev, NULL);
    }
    else if (ret == 0)
        close(fd);
}

static void Job_read_stderr_pipe(int fd, short flags, void *data) {
    assert(data != NULL);
    Job * job = data;

    char buf[1024];
    int ret = read(fd, buf, 1024);
    if (ret == -1)
        close(fd);
    else if (ret > 0) {
        buf[ret] = 0;
        Job_grow_and_append(&(job->stderr_output), buf, ret);
        struct event * stderr_ev = event_new(event_base, fd, EV_READ, Job_read_stderr_pipe, (void*) job);
        event_add(stderr_ev, NULL);
    }
    else if (ret == 0)
        close(fd);
}

int Job_setup_pipes(Job *job) {
    assert(job != NULL);
    struct event * stderr_ev = event_new(event_base, job->stderr_fd,
            EV_READ, Job_read_stderr_pipe, (void*) job);


    struct event * stdout_ev = event_new(event_base, job->stdout_fd,
            EV_READ, Job_read_stdout_pipe, (void*) job);

    event_add(stdout_ev, NULL);
    event_add(stderr_ev, NULL);

    return 1;
}

char * Job_to_json(Job *job) {
    json_object * jobj = json_object_new_object();
    json_object * jcmd = json_object_new_string(job->command);
    json_object * jpath = json_object_new_string(job->path);

    if (job->stdout_output != NULL) {
        json_object * jstdout = json_object_new_string(job->stdout_output);
        if (jstdout != NULL)
            json_object_object_add(jobj,"stdout", jstdout);
    }

    if (job->stderr_output != NULL) {
        json_object * jstderr = json_object_new_string(job->stderr_output);
        if (jstderr != NULL)
            json_object_object_add(jobj,"stderr", jstderr);
    }

    json_object *type = json_object_new_string("completion");
    json_object *jid = json_object_new_int(job->id);
    json_object *jnodeid = json_object_new_int(config->node_id);
    json_object *jexit_code = json_object_new_int(job->exit_code);
    json_object_object_add(jobj,"command", jcmd);
    json_object_object_add(jobj,"path", jpath);
    json_object_object_add(jobj,"id", jid);
    json_object_object_add(jobj,"node_id", jnodeid);
    json_object_object_add(jobj,"exit_code", jexit_code);
    json_object_object_add(jobj,"message_type", type);
    return (char *) json_object_to_json_string(jobj);
}
