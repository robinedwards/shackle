#include <event2/event.h>

typedef struct {
    int port;
    struct evconnlistener *listener;
    struct sockaddr_in sin;
} EventServer;

EventServer * EventServer_create(char *interface, int port);
void EventServer_run(EventServer *server);
