#include "event_server.h"
#include "job_manager.h"
#include "config.h"
#include <stdlib.h>

Config *config;

int main() {
    config = Config_create();
    JobManager_create(config->queue_size);
    printf("Listening on port %s:%d\n", config->interface, config->port);
    EventServer* s = EventServer_create(config->interface, config->port);
    EventServer_run(s);
    JobManager_destroy();
    return 0;
}
