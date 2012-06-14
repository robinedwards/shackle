#include <stdio.h>
#include <sys/socket.h>
#include "listen_server.h"
#include "event_loop.h"


static int read_callback (EventLoop *loop, int fd) {
    size_t bytes;
    char buffer[1024];

    bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if(bytes > 0 ) {
        buffer[bytes+1] = 0;
        printf("%d - %s\n", loop->nfds, buffer);
    }

    return 0;
}

static int accept_callback (EventLoop *loop, int fd) {
    printf("Accept\n");

    struct sockaddr_storage client_address;
    socklen_t address_length = sizeof(client_address);

    int new_fd = accept(fd, (struct sockaddr *) &client_address, &address_length);

    EventLoop_register_fd(loop, new_fd, 'r', &read_callback);
    return 0;
}

int main(void) {
    ListenServer *server = ListenServer_create(8008);
    int listen_fd        = ListenServer_start(server);
    EventLoop *loop      = EventLoop_create();

    EventLoop_register_fd(loop, listen_fd, 'r', &accept_callback);
    EventLoop_run(loop);
    return 0;
}
