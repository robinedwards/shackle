#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *command;
    char *std_output;
    char *std_error;

    int id;
    int time_limit;
    int state;
} Job;

Job *Job_create(int id, char *command, int time_limit) {
    assert(command != NULL);

    Job *new_job = malloc(sizeof(Job));
    assert(new_job != NULL);

    job->command = strdup(command);
    job->id = id;
    job->id = id;

    return job;
}

void Job_destroy(Job *who) {
    assert(who != NULL);

    free(who->name);
    free(who);
}

void Job_print(Job *who) {
    assert (who != NULL);
    printf("Name: %s, Age: %d, Height: %d Weight: %d",
            who->name,
            who->age,
            who->height,
            who->weight
          );
}

int main(int argc, char *argv[]) {
    Job *jim;

    jim = Job_create(
            "Jim",
            32,
            6,
            11
            );

    Job_print(jim);
    Job_destroy(jim);
    return 0;
}
