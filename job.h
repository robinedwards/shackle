#include <event2/event.h>

typedef struct {
    char *command;
    char *path;
    char *argv[256];
    int argc;
    int id;
    int pid;
    int exit_code;
    int time_limit;
    int stderr_fd;
    int stdout_fd;
    struct event * stderr_ev;
    struct event * stdout_ev;
    char *stderr_output;
    char *stdout_output;
} Job;

Job *Job_create(int id, char *command, int time_limit);
void Job_destroy(Job *job);
void Job_setup_child_handler(void);
void Job_print(Job *job);
int Job_launch(Job *job);
int Job_setup_pipes(Job *job, struct event_base *base);
