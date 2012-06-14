#include <sys/select.h>

typedef struct _watcher Watcher;

typedef struct {
    fd_set  write_fds;
    fd_set  read_fds;
    fd_set  except_fds;
    Watcher *read_watchers;
    Watcher *write_watchers;
    Watcher *except_watchers;
    int     nfds;
} EventLoop;

struct _watcher {
    struct _watcher * next;
    int    fd;
    int    (*callback)(EventLoop *, int);
};

EventLoop * EventLoop_create(void);
void EventLoop_register_fd(EventLoop *loop, int fd, char mode, int (*callback)(EventLoop *, int));
void EventLoop_run(EventLoop *loop);
void EventLoop_destroy(EventLoop *loop);
