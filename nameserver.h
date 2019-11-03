#ifndef NS_H_
#define NS_H_

#include "bwio.h"
#include "ts7200.h"
#include "syscall.h"
#include "string.h"

typedef struct NS_server{
    char *buffer_receive;
    char *buffer_reply;
    char name[50][100];
    int filled[50];
    int count;
} NS_server;

typedef struct NS_message{
    int type;
    int tid;
    char* name;
} NS_message;

void ns_server_boot();
void ns_server_initial(NS_server *server, char*buf_rcv, char*buf_rpy);
void ns_server_registeras(NS_server *server, NS_message *message);
void ns_server_whois(NS_server *server, NS_message *message);
void ns_server_exit(NS_server *server, NS_message *message);

int RegisterAs(char *name);
int WhoIs(char *name);
int EndAs(char *name);
#endif
