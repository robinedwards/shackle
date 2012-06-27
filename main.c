#include "event_server.h"
#include "job.h"

int main(void) {
    Job_setup_child_handler();
    EventServer* s = EventServer_create(9090);
    EventServer_run(s);
    return 0;
}
