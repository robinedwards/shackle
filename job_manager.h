#include "job.h"

typedef struct {
    Job **queue;
    int queue_size;
} JobManager;


void JobManager_create(int size);
void JobManager_destroy();
int JobManager_add_job(Job *job);
