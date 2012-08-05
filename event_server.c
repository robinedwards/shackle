#include <arpa/inet.h>
#include <fcntl.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "event_server.h"
#include "die.h"
#include "message_handler.h"


struct event_base *event_base;


static void EventServer_write_callback(struct bufferevent *bev, void *arg) {
    bufferevent_free(bev);
}

static void EventServer_error_callback(struct bufferevent *bev, short error, void *ctx) {
    if (error & BEV_EVENT_ERROR) {
        die("An error occured %dn", error);
    } else if (error & BEV_EVENT_TIMEOUT) {
        die("Unhandled timeout event");
    }
    bufferevent_free(bev);
}

static void EventServer_read_callback(struct bufferevent *bev, void *arg) {
    EventServer *server = arg;
    struct evbuffer *input, *output;
    char *request;
    size_t n;
    input = bufferevent_get_input(bev);
    output = bufferevent_get_output(bev);

    while ((request = evbuffer_readln(input, &n, EVBUFFER_EOL_LF))) {
        char *response = message_handler(request, n, event_base);
        free(request);
        evbuffer_add(output, response, strlen(response)+1);
        bufferevent_setcb(bev, EventServer_read_callback, EventServer_write_callback, EventServer_error_callback, server);
    }
}


static void EventServer_accept_callback(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {
    EventServer * server = ctx;
    /* We got a new connection! Set up a bufferevent for it. */
    struct bufferevent *bev = bufferevent_socket_new(event_base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setwatermark(bev, EV_WRITE, 1, 9999);
    bufferevent_setcb(bev, EventServer_read_callback, NULL, EventServer_error_callback, server);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void EventServer_accept_error_callback(struct evconnlistener *listener, void *ctx) {
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));
    event_base_loopexit(event_base, NULL);
}

EventServer * EventServer_create(char * interface, int port) {
    assert(port > 0 && port < 65537);
    EventServer * server = malloc(sizeof(EventServer));
    assert(server != NULL);

    server->port = port;

    event_base = event_base_new();
    if (!event_base)
        return NULL;

    memset(&(server->sin), 0, sizeof(struct sockaddr_in));
    server->sin.sin_family = AF_INET;
    server->sin.sin_addr.s_addr = inet_addr(interface);
    server->sin.sin_port = htons(server->port);

    server->listener = evconnlistener_new_bind(event_base, EventServer_accept_callback, server,
            LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
            (struct sockaddr*)&(server->sin), sizeof(struct sockaddr_in));
    if (!server->listener)
        die("Couldn't create listener");
    evconnlistener_set_error_cb(server->listener, EventServer_accept_error_callback);
    return server;
}

void EventServer_run(EventServer *server) {
    event_base_dispatch(event_base);
}

// TODO EventServer_destory
