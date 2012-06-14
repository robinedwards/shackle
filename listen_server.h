typedef struct {
    int port;
    int listen_fd;
    int backlog; // max pending connections for socket
} ListenServer;

ListenServer * ListenServer_create(int port);
int ListenServer_start(ListenServer *server);
void ListenServer_destroy(ListenServer *server);
