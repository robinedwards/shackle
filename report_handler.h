#include <arpa/inet.h>

typedef struct {
    int port;
    char * host;
    char * content;
    struct event_base *event_base;
    char * report;
    struct sockaddr_in addr;
} ReportHandler;


ReportHandler * ReportHandler_create(int port, char *server, char * content, struct event_base *base);
void ReportHandler_destroy(ReportHandler *r);
int ReportHandler_send(ReportHandler *r);
