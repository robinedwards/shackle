typedef struct {
    char *command;
    char *std_output;
    char *std_error;
    int fd;
    int id;
    int pid;
    int exit_code;
    int time_limit;
    int stdin_fd;
    int stderr_fd;
    int stdout_fd;
} Job;

Job *Job_create(int id, char *command, int time_limit);
void Job_setup_child_handler(void);
void Job_destroy(Job *job);
void Job_print(Job *job);
int Job_launch(Job *job);
