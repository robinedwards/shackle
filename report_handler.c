#include "report_handler.h"
#include "die.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <stdio.h>

ReportHandler * ReportHandler_create(int port, char *host, char *content, struct event_base *base){
    assert(host != NULL);
    assert(base != NULL);
    assert(content != NULL);

    ReportHandler *report = malloc(sizeof(ReportHandler));
    report->event_base = base;
    report->host = host;
    report->port = port;
    report->content = content;

    memset(&(report->addr), 0, sizeof(report->addr));
    report->addr.sin_family = AF_INET;
    report->addr.sin_port = htons(report->port);
    report->addr.sin_addr.s_addr = inet_addr(report->host);
    return report;
}

void ReportHandler_destroy(ReportHandler *r) {
    assert(r != NULL);
    // free(r->content);
    // free(r->content);
    free(r);
}

static void ReportHandler_write_callback(struct bufferevent *bev, void *arg) {
    ReportHandler *r = arg;
    bufferevent_free(bev);
    ReportHandler_destroy(r);
}

static void ReportHandler_callback(struct bufferevent *bev, short events,  void *arg) {
    ReportHandler *r = arg;
    if (events & BEV_EVENT_CONNECTED) {
        bufferevent_write(bev, r->content, sizeof(char) * strlen(r->content));
    } else if (events & BEV_EVENT_ERROR)
        die("Error connecting to master (cb)");
}

int ReportHandler_send(ReportHandler * report) {
    struct bufferevent *bev = bufferevent_socket_new(report->event_base, -1, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, NULL, ReportHandler_write_callback, ReportHandler_callback, report);
    if(bufferevent_socket_connect(bev, (struct sockaddr *) &(report->addr), sizeof(report->addr)) < 0)
        die("Couldn't connect to master");

    return 0;
}
