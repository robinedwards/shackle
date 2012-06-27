#include <event2/event.h>

typedef struct {
    int port;
    int free_socket;
    evutil_socket_t listener;
    struct sockaddr_in sin;
    struct event_base *base;
    struct event *listener_event;
} EventServer;

EventServer * EventServer_create(int port);
void EventServer_run(EventServer *server);
