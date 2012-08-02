#include "event_server.h"
#include "job_manager.h"
#include <stdlib.h>

int main(int argc, char **argv) {
    int port = atoi(argv[1]);
    JobManager_create(10);
    printf("Listening on port %d\n", port);
    EventServer* s = EventServer_create(port);
    EventServer_run(s);
    JobManager_destroy();
    return 0;
}
