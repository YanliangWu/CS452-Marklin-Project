#include "nameserver.h"

void ns_server_boot() {
    //bwprintf(COM2,"ns_boot start\n\r");

    NS_server server;
    NS_message *message_receive;
    // NS_message *message_send;
    int message_size = sizeof(NS_message);
    char buffer_receive[message_size];
    char buffer_reply[message_size];
    ns_server_initial(&server,buffer_receive,buffer_reply);
    int tid;
    while(1){
        //bwprintf(COM2, "NameServer: Receiving....\n\r");
        Receive(&tid, server.buffer_receive, message_size);
        //bwprintf(COM2, "NameServer: Received....\n\r");
        message_receive = (NS_message *) server.buffer_receive;

        switch(message_receive->type){
            case 2:
                {
                    ns_server_registeras(&server, message_receive);
                    break;   
                }
            case 3:
                {
                    ns_server_whois(&server, message_receive);
                    break;
                }
            case 4:
                {
                    ns_server_exit(&server, message_receive);
                    break;
                }
        }
        Pass(); 
    }
}


void ns_server_initial(NS_server *server, char*buf_rcv, char*buf_rpy){
    //bwprintf(COM2, "ns_server_initital start\n\r");
    server->count = 0;
    server->buffer_receive = buf_rcv;
    server->buffer_reply = buf_rpy;
    int i;
    for(i=1;i<50;i++){
        server->filled[i] = 0;
    }
    //bwprintf(COM2, "ns_server_initital ends\n\r");
}

void ns_server_registeras(NS_server *server, NS_message *message){  
    int message_size = sizeof(NS_message);
    int tid = message->tid;
    stringcpy(server->name[tid], message->name, message_size);
    server->filled[tid] = 1;

    NS_message *message_reply = (NS_message *) server->buffer_reply;
    message_reply->type = 1;

    Reply(tid, server->buffer_reply, message_size);
}

void ns_server_whois(NS_server *server, NS_message *message){
    int message_size = sizeof(NS_message);
    int tid;
    for(tid=1;tid<50;tid++){
        if(!server->filled[tid])
            continue;
        if(!stringcmp(server->name[tid], message->name,message_size))
            break;
    }

    NS_message *message_reply = (NS_message *) server->buffer_reply;
    message_reply->type = 1;
    message_reply->tid = tid;
    //bwprintf(COM2,"whois tid: %d\n\r",message->tid);
    Reply(message->tid, server->buffer_reply, message_size);
}

void ns_server_exit(NS_server *server, NS_message *message){
    NS_message *message_reply = (NS_message *) server->buffer_reply;
    message_reply->type = 4;
    message_reply->tid = message->tid;
    server->filled[message->tid] = 0;
    Reply(message->tid, server->buffer_reply, sizeof(NS_message));
}

int RegisterAs (char *name){
    NS_message *message_send;
    NS_message *message_receive;
    int message_size = sizeof(NS_message);
    char buffer_send[message_size];
    char buffer_receive[message_size];

    message_send = (NS_message *) buffer_send;
    message_send->type = 2;
    message_send->tid = MyTid();
    message_send->name = name;

    Send(1,buffer_send, message_size, buffer_receive, message_size);

    message_receive = (NS_message *) buffer_receive;

    //bwprintf(COM2,"REGISTER AS\n\r");
    return message_receive->type;
}

int WhoIs (char *name){
    NS_message *message_send;
    NS_message *message_receive;
    int message_size = sizeof(NS_message);
    char buffer_send[message_size];
    char buffer_receive[message_size];

    message_send = (NS_message *) buffer_send;
    message_send->type = 3;
    message_send->tid = MyTid();
    message_send->name = name;
    //bwprintf(COM2,"flag tid:%d\n\r",message_send->tid);

    Send(1,buffer_send, message_size, buffer_receive, message_size);

    message_receive = (NS_message *) buffer_receive;

//    bwprintf(COM1,"WHOIS: %d\n\r",message_receive->tid);
    return message_receive->tid;
}

int EndAs(char *name){
    NS_message *message_send;
    NS_message *message_receive;
    int message_size = sizeof(NS_message);
    char buffer_send[message_size];
    char buffer_receive[message_size];
    
    message_send = (NS_message *) buffer_send;
    message_send->type = 4;
    message_send->tid = MyTid();
    message_send->name = name;
    Send(1, buffer_send, message_size, buffer_receive, message_size); 
    message_receive = (NS_message *) buffer_receive;
    
    //bwprintf(COM2,"ENDAS %d\n\r",message_send->tid);
    return 1; 
}
