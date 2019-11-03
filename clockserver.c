#include "clockserver.h"
#include "trainserver.h"

//----------------------------------server--------------------------------
void clock_server_boot(){
    //create server
    char server_name[] = "CL\000";
    RegisterAs(server_name);
    clock_server server;
    clock_server_initial(&server,MyTid());
    Create(2,&clock_notifier_boot); //5
    //initial timer
	clock_server_settime();

    while(1){
        Receive(&server.client_tid, server.buf_rcv, server.mes_size);
        switch(server.mes_rcv->type){
            case CL_NOTIFIER:{
                clock_server_notifier_handle(&server);
                break;
            }
            case CL_DELAY_REQUEST:{
                clock_server_delay_handle(&server);
                break;
            }
            case CL_TIME_REQUEST:{
                clock_server_time_handle(&server);
                break;
            }
        }
        clock_server_unblock(&server);
    }
}

//----------------------initial server--------------------------
void clock_server_settime(){

	int * timer_ldr = (int *)(TIMER3_BASE + LDR_OFFSET);
    int * timer_ctrl = (int *)(TIMER3_BASE + CRTL_OFFSET);
    *timer_ctrl = (*timer_ctrl) ^ ENABLE_MASK;
    *timer_ldr = 50800;
    *timer_ctrl = ENABLE_MASK | CLKSEL_MASK | MODE_MASK;
}

void clock_server_initial(clock_server *server, int server_tid){
    server->server_tid = server_tid;
    server->mes_send = (CL_message*) server->buf_send;
    server->mes_rcv = (CL_message*) server->buf_rcv;
    server->mes_rpy = (CL_message*) server->buf_rpy;
    server->mes_size = sizeof(CL_message);
    server->tick = 0;
    int i;
    for(i=1;i<50;i++)
        server->task_delay[i] = -1;
}

//-----------------------server case handle-------------------------
void clock_server_notifier_handle(clock_server *server){
    server->tick++;
    Reply(server->client_tid,server->buf_rpy,server->mes_size);


    int message_size2 = sizeof(TR_message);
    char buffer_send[message_size2];
    char buffer_reply[message_size2];
    TR_message *message_send2 = (TR_message *) buffer_send;
    message_send2->type = TR_CLOCK;
    Send(4,buffer_send,message_size2,buffer_reply,message_size2);


}

void clock_server_delay_handle(clock_server *server){
    int tick = server->mes_rcv->num;
    server->task_delay[server->client_tid] = server->tick + tick;
}

void clock_server_time_handle(clock_server *server){
    server->mes_rpy->num = server->tick;
    Reply(server->client_tid,server->buf_rpy,server->mes_size);
}

//-------------------------unblock-------------------------
void clock_server_unblock(clock_server *server){
    int i;
    for(i=1;i<50;i++){
        if(server->tick == server->task_delay[i]){
            server->mes_rpy->type = CL_DELAY_REQUEST;
            Reply(i,server->buf_rpy,server->mes_size);
            server->task_delay[i] = -1;
        }
    }
}


//-----------------------notifier-------------------------
void clock_notifier_boot () {
    clock_server server;
    clock_server_initial(&server,MyParentTid());
    server.mes_send->type = CL_NOTIFIER;
    while(1) {
        AwaitEvent(CLOCK);
        Send(server.server_tid,server.buf_send,server.mes_size,\
            server.buf_rcv,server.mes_size);
    }
}

//-------------------------idle-----------------------------
void clock_idle_boot () {
    while(1) {
    }
}


//---------------------application------------------------
void Delay(clock_server *server, int tick){
    server->mes_send->type = CL_DELAY_REQUEST;
    server->mes_send->num = tick;
    Send(server->server_tid, server->buf_send, server->mes_size,\
        server->buf_rpy,server->mes_size);
}

int Time(clock_server *server){
    server->mes_send->type = CL_TIME_REQUEST;
    Send(server->server_tid, server->buf_send, server->mes_size,\
        server->buf_rpy,server->mes_size);
    return server->mes_rpy->num;
}