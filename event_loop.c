#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/select.h>
#include "die.h"
#include "event_loop.h"

/* add a new watcher */
static Watcher *Watcher_add(Watcher **start, int fd, int (*callback)(EventLoop*, int)) {
    assert(start != NULL);

    Watcher *new_watcher= malloc(sizeof(Watcher));
    assert(new_watcher != NULL);

    new_watcher->fd       = fd;
    new_watcher->callback = callback;
    new_watcher->next     = NULL;

    // empty list
    if (*start == NULL) {
        *start = new_watcher;
    }
    else {
        // add to end of list
        Watcher *pos = *start;
        while (pos != NULL)
            pos = pos->next;
        pos = new_watcher;
    }
    return new_watcher;
}

static void Watcher_destroy(Watcher *start) {
    assert(start != NULL);

    Watcher *pos = start;
    Watcher *cur = pos;
    while (pos != NULL) {
        cur = pos;
        free(cur);
        pos = pos->next;
    }
}

EventLoop * EventLoop_create(void) {
    EventLoop *loop = malloc(sizeof(EventLoop));
    assert(loop != NULL);

    FD_ZERO(&(loop->write_fds));
    FD_ZERO(&(loop->read_fds));
    FD_ZERO(&(loop->except_fds));

    loop->nfds            = 0;
    loop->read_watchers   = NULL;
    loop->write_watchers  = NULL;
    loop->except_watchers = NULL;

    return loop;
}

void EventLoop_register_fd(EventLoop *loop, int fd, char mode, int (*callback)(EventLoop *, int)) {
    assert(loop != NULL);

    switch (mode) {
        case 'r':
            FD_SET(fd, &(loop->read_fds));
            Watcher_add(&(loop->read_watchers), fd, callback);
            break;
        case 'w':
            FD_SET(fd, &(loop->write_fds));
            Watcher_add(&(loop->write_watchers), fd, callback);
            break;
        case 'e':
            FD_SET(fd, &(loop->except_fds));
            Watcher_add(&(loop->except_watchers), fd, callback);
            break;
        default:
            die("Invalid mode %c, valid modes: r, w, e", mode);
    }

    if (fd > loop-> nfds)
        loop->nfds = fd;
}

static void EventLoop_check_watchers(EventLoop *loop, fd_set *fds, Watcher *start) {
    if (start == NULL)
        return;

    Watcher *pos = start;

    while (pos != NULL) {
        if (FD_ISSET(pos->fd, fds)) {
            (pos->callback)(loop, pos->fd);
        }
        pos = pos->next;
    }
}

void EventLoop_run(EventLoop *loop) {
    assert(loop != NULL);

    while (1) {
        int retval = select(loop->nfds+1, &(loop->read_fds), &(loop->write_fds),
                &(loop->except_fds), NULL);

        if(retval == -1) {
            perror("select error");
            exit(4);
        }
        else if (retval) {
            printf("data available, %d\n", retval);
            EventLoop_check_watchers(loop, &(loop->read_fds), loop->read_watchers);
            EventLoop_check_watchers(loop, &(loop->write_fds), loop->write_watchers);
            EventLoop_check_watchers(loop, &(loop->except_fds), loop->except_watchers);
        }
        else {
            printf("no data available\n");
        }
    }
}

void EventLoop_destroy(EventLoop *loop) {
    assert(loop != NULL);

    if (loop->read_watchers != NULL)
        Watcher_destroy(loop->read_watchers);
    if (loop->write_watchers != NULL)
        Watcher_destroy(loop->write_watchers);
    if (loop->except_watchers != NULL)
        Watcher_destroy(loop->except_watchers);

    free(loop);
}
