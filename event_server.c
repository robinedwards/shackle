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
#include "event_server.h"
#include "die.h"
#include "message_handler.h"

static void EventServer_read_callback(struct bufferevent *bev, void *arg) {
    EventServer *server = arg;
    struct evbuffer *input, *output;
    char *request;
    size_t n;
    input = bufferevent_get_input(bev);
    output = bufferevent_get_output(bev);

    while ((request = evbuffer_readln(input, &n, EVBUFFER_EOL_LF))) {
        char *response = message_handler(request, n);
        free(request);
        evbuffer_add(output, response, strlen(response)+1);
    }
    server->free_socket = 1;
}

static void EventServer_write_callback(struct bufferevent *bev, void *arg) {
    EventServer *server = arg;
    if (server->free_socket) {
        bufferevent_free(bev);
        server->free_socket = 0;
    }
}

static void EventServer_error_callback(struct bufferevent *bev, short error, void *ctx) {
    if (error & BEV_EVENT_ERROR) {
        die("An error occured %dn", error);
    } else if (error & BEV_EVENT_TIMEOUT) {
        die("Unhandled timeout event");
    }
    bufferevent_free(bev);
}

static void ListenServer_accept(evutil_socket_t listener, short event, void *arg) {
    EventServer *server = arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        die("accept failed");
    } else if (fd > FD_SETSIZE) {
        close(fd);
    } else {
        struct bufferevent *bev;
        evutil_make_socket_nonblocking(fd);
        bev = bufferevent_socket_new(server->base, fd, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setwatermark(bev, EV_WRITE, 1, 9999);
        bufferevent_setcb(bev, EventServer_read_callback, EventServer_write_callback, EventServer_error_callback, server);
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

// TODO EventServer_destory
