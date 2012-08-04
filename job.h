#include <event2/event.h>

typedef struct {
    char *command;
    char *path;
    char *argv[256];
    int argc;
    int id;
    int pid;
    int exit_code; // TODO
    int time_limit; // TODO
    int stderr_fd;
    int stdout_fd;
    char *stderr_output;
    char *stdout_output;
    struct event_base *event_base;
} Job;

Job *Job_create(int id, char *command, int time_limit, struct event_base *base);
void Job_destroy(Job *job);
void Job_setup_child_handler(void);
void Job_print(Job *job);
int Job_launch(Job *job);
int Job_setup_pipes(Job *job);
