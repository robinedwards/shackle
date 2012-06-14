#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE 16384

static char * EventServer_handler(char *request, int n) {
    char *x = "BLAH\n";
    return x;
}

static void EventServer_read_callback(struct bufferevent *bev, void *args) {
    struct evbuffer *input, *output;
    char *request;
    size_t n;
    input = bufferevent_get_input(bev);
    output = bufferevent_get_output(bev);

    while ((request = evbuffer_readln(input, &n, EVBUFFER_EOL_LF))) {
        char *response = EventServer_handler(request, n);
        free(request);
        evbuffer_add(output, response, strlen(response)+1);
    }
}

void EventServer_error_callback(struct bufferevent *bev, short error, void *ctx) {
    if (error & BEV_EVENT_EOF) {
        /* connection has been closed, do any clean up here */
        /* ... */
    } else if (error & BEV_EVENT_ERROR) {
        /* check errno to see what error occurred */
        /* ... */
    } else if (error & BEV_EVENT_TIMEOUT) {
        /* must be a timeout event handle, handle it */
        /* ... */
    }
    bufferevent_free(bev);
}

typedef struct {
    int port;
    evutil_socket_t listener;
    struct sockaddr_in sin;
    struct event_base *base;
    struct event *listener_event;
} EventServer;

static void ListenServer_accept(evutil_socket_t listener, short event, void *arg) {
    EventServer *server = arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        perror("accept");
    } else if (fd > FD_SETSIZE) {
        close(fd);
    } else {
        struct bufferevent *bev;
        evutil_make_socket_nonblocking(fd);
        bev = bufferevent_socket_new(server->base, fd, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, EventServer_read_callback, NULL, EventServer_error_callback, NULL);
        bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
        bufferevent_enable(bev, EV_READ|EV_WRITE);
    }
}

EventServer * EventServer_create(int port) {
    assert(port > 0);
    EventServer * server = malloc(sizeof(EventServer));
    assert(server != NULL);

    server->port = port;

    server->base = event_base_new();
    if (!server->base)
        return NULL;

    server->sin.sin_family = AF_INET;
    server->sin.sin_addr.s_addr = 0;
    server->sin.sin_port = htons(server->port);

    server->listener = socket(AF_INET, SOCK_STREAM, 0);
    evutil_make_socket_nonblocking(server->listener);
    int one = 1;
    setsockopt(server->listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    return server;
}

void EventServer_run(EventServer *server) {
    struct event *listener_event;
    assert(server != NULL);

    if (bind(server->listener, (struct sockaddr*)&server->sin, sizeof(server->sin)) < 0) {
        perror("Failed to bind");
        return;
    }

    if (listen(server->listener, 16) < 0) {
        perror("Failed to listen");
        return;
    }

    listener_event = event_new(server->base, server->listener,
            EV_READ|EV_PERSIST, ListenServer_accept, (void*)server);

    assert(listener_event != NULL);
    event_add(listener_event, NULL);
    event_base_dispatch(server->base);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    EventServer* s = EventServer_create(9090);
    EventServer_run(s);
    return 0;
}
