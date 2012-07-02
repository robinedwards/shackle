#include "event_server.h"
#include "job_manager.h"

int main(void) {
    JobManager_create(10);
    EventServer* s = EventServer_create(9090);
    printf("Listening on port 9090\n");
    EventServer_run(s);
    JobManager_destroy();
    return 0;
}
