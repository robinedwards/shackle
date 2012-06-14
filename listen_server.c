#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "listen_server.h"
#include "die.h"

ListenServer * ListenServer_create(int port) {
    assert(port > 0);

    ListenServer *server = malloc(sizeof(ListenServer));
    assert(server != NULL);

    server->port = port;
    server->backlog = 10;

    return server;
}

static void ListenServer_bind(ListenServer *server) {
    assert(server != NULL);
    struct addrinfo hints, *ai_list, *ai;

    // setup hints for getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    char port[256];
    if(sprintf(port, "%d", server->port) == 0)
        die("couldn't convert port to string!");

    // getaddrinfo list
    int r;
    if ((r = getaddrinfo(NULL, port, &hints, &ai_list)) != 0)
        die(gai_strerror(r));

    int listen_fd;

    // loop through ai_list & attempt bind
    for (ai = ai_list; ai != NULL; ai = ai_list->ai_next) {
        listen_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (listen_fd < 0) continue;

        // allow reuse, see man 7 socket
        int y = 1;
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int)) != 0)
            die("Couldn't set socket options");

        if (bind(listen_fd, ai->ai_addr, ai->ai_addrlen) < 0) {
            close(listen_fd);
            continue;
        }

        break;
    }

    if(ai == NULL) die("Failed to bind on port %d", server->port);
    server->listen_fd = listen_fd;
    freeaddrinfo(ai_list);
}

int ListenServer_start(ListenServer *server) {
    assert(server != NULL);

    printf("Trying to bind to port %d\n", server->port);
    ListenServer_bind(server);
    printf("Bound to port %d\n", server->port);

    if (listen(server->listen_fd, server->backlog) == -1)
        die("couldn't listen");

    printf("Listening on port %d\n", server->port);

    return server->listen_fd;
}

void ListenServer_destroy(ListenServer *server) {
    assert(server != NULL);

    if (server->listen_fd > 0)
        close(server->listen_fd);

    free(server);
}
