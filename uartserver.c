// I/O server....

#include "uartserver.h"
#include "clockserver.h"
#include "trainserver.h"
#include "kernel_hwi.h"
#include "kernel_swi.h"
#include "print.h"
#define DOUZI_DEBUG 0
#define COM 1
void buffer_init(BD *buf, char *str, int size){
    buf->count = 0;
    buf->start = 0;
    buf->end = 0;
    buf->size = size;
    buf->Buffer = str;
}

void buffer_put(BD *buf, char c){
    buf->Buffer[buf->end] = c;
    buf->end = (buf->end+1)%buf->size;
    buf->count++;
    if(buf->count==buf->size)
        bwprintf(COM2,"buffer overflow--------1 size:%d\n\r",buf->size);
}

void buffer_puts(BD *buf, char *c){
    int i=0;
    while(c[i]!='\000') {
        buffer_put(buf, c[i]);
        i++;
    }
}

char buffer_get(BD *buf){
    char item = 0;
    if(buf->count>0){
        item = buf->Buffer[buf->start];
        buf->start = (buf->start+1)%buf->size;
        buf->count--;
    }
    return item;
}

void uart_server_boot(){

    char server_name[] = "US\000";
    RegisterAs(server_name);

    // create the notifier....
    Create(5,&uart1_input_notifier_boot);       //7
    Create(4,&uart1_output_notifier_boot);      //8
    Create(6,&uart2_input_notifier_boot);       //9
    Create(6,&uart2_output_notifier_boot);      //10
    char uart1_out_name[] = "1O\000";
    char uart2_out_name[] = "2O\000";
    int uart1out = WhoIs(uart1_out_name);
    int uart2out = WhoIs(uart2_out_name);
    // create a lots of buffer...
    char u1trStr[U1TR_BUF_SIZE];
    char u2trStr[U2TR_BUF_SIZE];
    char u1rcStr[U1RC_BUF_SIZE];
    char u2rcStr[U2RC_BUF_SIZE];
    char comStr[COM_BUF_SIZE];


    BD uart1trBuff;           // uart1 transmit buffer
    BD uart2trBuff;          // uart2 transmit buffer
    BD uart1rcBuff;         // uart1 receive buffer
    BD uart2rcBuff;        // uart2 receive buffer
    BD commandBuff;

    buffer_init(&uart1trBuff,u1trStr,U1TR_BUF_SIZE);
    buffer_init(&uart2trBuff,u2trStr,U2TR_BUF_SIZE);
    buffer_init(&uart1rcBuff,u1rcStr,U1RC_BUF_SIZE);
    buffer_init(&uart2rcBuff,u2rcStr,U2RC_BUF_SIZE);
    buffer_init(&commandBuff,comStr,COM_BUF_SIZE);

    // Initialize server
    uart_server server;
    int server_tid = MyTid();
    uart_server_initial(&server,server_tid);

    // Declear Variables
    short Uart1NotifierReady = 0;
    short Uart2NotifierReady = 0;
    short getuart1 = 0;
    short getuart2 = 0;

    while(1){
        Receive(&server.client_tid, server.buf_rcv, server.mes_size);
        if(server.mes_rcv->type>=12 || server.mes_rcv->type<=0)
            bwprintf(COM, "UART_TYPE:%d \n\r",server.mes_rcv->type);
        switch(server.mes_rcv->type){

            case UART_UART1_IN:{ //2
                char c = server.mes_rcv->buffer;
                Reply(server.client_tid, server.buf_rpy, server.mes_size);
               // bwprintf(COM2, "Sensor Data received: %d \n\r", c);
                if(getuart1){
                    server.mes_rpy->type = UART_UART1_IN;
                    server.mes_rpy->buffer = c;
                    int reply_tid = getuart1;
                    getuart1 = 0;
                    Reply(reply_tid, server.buf_rpy, server.mes_size);
                }
                else{
                    buffer_put(&uart1rcBuff, c);
                }
                break;}

            case UART_UART2_IN:{ //3
                if(DOUZI_DEBUG) bwprintf(COM, "UART_UART2_IN.....\n\r");
                int c = server.mes_rcv->buffer;
                //bwprintf(COM2, "SHOULD reply 9....(%d)\n\r",server.client_tid);
                Reply(server.client_tid, server.buf_rpy, server.mes_size);
                uart_server_print_command(&server,&commandBuff,\
                    &uart2trBuff,&Uart2NotifierReady,uart2out);//
                if(getuart2){
                    server.mes_rpy->type = UART_UART2_IN;
                    server.mes_rpy->buffer = c;
                    int reply_tid = getuart2;
                    getuart2 = 0;
                    if(DOUZI_DEBUG)bwprintf(COM, "Replying get uart 2 notifier...\n\r");
                    Reply(reply_tid, server.buf_rpy, server.mes_size);
                }
                else{
                    if(DOUZI_DEBUG)bwprintf(COM, "ELSE...\n\r");
                    buffer_put(&uart2rcBuff,c);
                }
                break;}

            case UART_UART1_OUT:{ //4
                if(!uart1trBuff.count){
                    Uart1NotifierReady = 1;
                }
                else{
                    server.mes_rpy->buffer = buffer_get(&uart1trBuff);
                   // bwprintf(COM2, "%d sending to uart1\n\r", server.mes_rpy->buffer);
                    Reply(server.client_tid, server.buf_rpy, server.mes_size);
                }
                break;}

            case UART_UART2_OUT:{ //5
                if(!uart2trBuff.count){
                    if(DOUZI_DEBUG)bwprintf(COM,"UART2 notifier ready!!\n\r");
                    Uart2NotifierReady = 1;
                }
                else{
                    server.mes_rpy->buffer = buffer_get(&uart2trBuff);
                    if(DOUZI_DEBUG)bwprintf(COM,"UART2 transmitting: %c\n\r",server.mes_rpy->buffer);
                    Reply(server.client_tid, server.buf_rpy, server.mes_size);
                }
                break;}

            case UART_PUT_UART1:{ //6
                if(Uart1NotifierReady){
                    buffer_put(&uart1trBuff,server.mes_rcv->buffer);
                    server.mes_rpy->buffer = buffer_get(&uart1trBuff);
                    Reply(uart1out, server.buf_rpy, server.mes_size);
                    Uart1NotifierReady = 0;
                }
                else{
                    buffer_put(&uart1trBuff,server.mes_rcv->buffer);
                }
                Reply(server.client_tid,server.buf_rpy, server.mes_size);
                break;}

            case UART_PUT_UART2:{ //7
                if(Uart2NotifierReady){
                    buffer_put(&uart2trBuff,server.mes_rcv->buffer);
                    server.mes_rpy->buffer = buffer_get(&uart2trBuff);
                    Reply(uart2out, server.buf_rpy, server.mes_size);
                    Uart2NotifierReady = 0;
                }
                else{
                    buffer_put(&uart2trBuff,server.mes_rcv->buffer);
                }
                Reply(server.client_tid,server.buf_rpy, server.mes_size);
                break;}

            case UART_PUT_UART1_STR:{ //8
            //    uart_server_print_debug(&server,1,8,&uart2trBuff,&Uart2NotifierReady,uart2out);
                buffer_puts(&uart1trBuff,server.mes_rcv->str);
                if(Uart1NotifierReady){
                    server.mes_rpy->buffer = buffer_get(&uart1trBuff);
                    Reply(uart1out, server.buf_rpy, server.mes_size);
                    Uart1NotifierReady = 0;
                }
                Reply(server.client_tid,server.buf_rpy, server.mes_size);
                break;}

            case UART_PUT_UART2_STR:{ //9
               // uart_server_print_debug(&server,1,8,&uart2trBuff,&Uart2NotifierReady,uart2out);
                buffer_puts(&uart2trBuff,server.mes_rcv->str);
                if(Uart2NotifierReady){
                    server.mes_rpy->buffer = buffer_get(&uart2trBuff);
                    Reply(uart2out, server.buf_rpy, server.mes_size);
                    Uart2NotifierReady = 0;
                }
                Reply(server.client_tid,server.buf_rpy, server.mes_size);
                break;}

            case UART_GET_UART1:{ //10
              //  uart_server_print_debug(&server,1,10,&uart2trBuff,&Uart2NotifierReady,uart2out);
                if(uart1rcBuff.count){
                    server.mes_rpy->type = UART_GET_UART1;
                    server.mes_rpy->buffer = buffer_get(&uart1rcBuff);
                    Reply(server.client_tid, server.buf_rpy, server.mes_size);
                }
                else{
                    if(DOUZI_DEBUG)bwprintf(COM, "UART_GET_UART1: blocked.....\n\r");
                    getuart1 = server.client_tid;
                }
                break;}

            case UART_GET_UART2:{ //11
             //   uart_server_print_debug(&server,1,11,&uart2trBuff,&Uart2NotifierReady,uart2out);
                if(uart2rcBuff.count){
                    server.mes_rpy->type = UART_GET_UART2;
                    server.mes_rpy->buffer = buffer_get(&uart2rcBuff);
                    Reply(server.client_tid, server.buf_rpy, server.mes_size);
                }
                else{
                    if(DOUZI_DEBUG)bwprintf(COM, "UART_GET_UART2 Blocked.......\n\r");
                    getuart2 = server.client_tid;
                }
                break;}
            default:{
                break;
            }

        }
    }
}

void uart_server_initial(uart_server *server, int server_tid){
    server->server_tid = server_tid;
    server->mes_send = (UART_message*) server->buf_send;
    server->mes_rcv = (UART_message*) server->buf_rcv;
    server->mes_rpy = (UART_message*) server->buf_rpy;
    server->mes_size = sizeof(UART_message);
}


void uart_server_print_command(uart_server *server, BD *com, BD *buf,\
    short *Uart2NotifierReady,int uart2out){
    char c = server->mes_rcv->buffer;
    if(c == '\n' || c == '\r'){com->end = 0;}
    else if(c == '\b'){if(com->end>0)com->end--;}
    else{com->Buffer[com->end++] = c;}
    buffer_puts(buf,"\033[17;8H\000");
    int i;
    for(i=0;i<com->end;i++)
        buffer_put(buf,com->Buffer[i]);
    for(i=com->end;i<40;i++)
        buffer_put(buf,' ');
    if(*Uart2NotifierReady){
        server->mes_rpy->buffer = buffer_get(buf);
        Reply(uart2out, server->buf_rpy, server->mes_size);
        *Uart2NotifierReady = 0;
    }

}


//------------------------------notifier---------------------------------
void uart1_input_notifier_boot(){
    int *data = (int *)( UART1_BASE + UART_DATA_OFFSET );
    char name[] = "1I\000";
    RegisterAs(name);
    uart_server server;
    uart_server_initial(&server,MyParentTid());
    server.mes_send->type = UART_UART1_IN;
    while(1){
        AwaitEvent(UART1_IN);
        server.mes_send->buffer = *data;
        Send(3,server.buf_send,server.mes_size,\
            server.buf_rpy,server.mes_size);
    }
}

void uart1_output_notifier_boot(){
    int *data = (int *)( UART1_BASE + UART_DATA_OFFSET );
    char name[] = "1O\000";
    RegisterAs(name);
    uart_server server;
    uart_server_initial(&server,MyParentTid());
    server.mes_send->type = UART_UART1_OUT;
    while(1){
        AwaitEvent(UART1_OUT);
        Send(3,server.buf_send,server.mes_size,\
            server.buf_rpy,server.mes_size);
       // bwprintf(COM2, "\033[40;8H SENDING %d train....",server.mes_rpy->buffer);
        *data = server.mes_rpy->buffer;
    }
}


void uart2_input_notifier_boot(){
    int *data = (int *)( UART2_BASE + UART_DATA_OFFSET );
    char name[] = "2I\000";
    RegisterAs(name);
    uart_server server;
    uart_server_initial(&server, MyParentTid());
    server.mes_send->type = UART_UART2_IN;
    while(1){
        AwaitEvent(UART2_IN);
        server.mes_send->buffer = *data;
        Send(3,server.buf_send,server.mes_size,\
            server.buf_rpy,server.mes_size);
    }
}


void uart2_output_notifier_boot(){
    int *data = (int *)( UART2_BASE + UART_DATA_OFFSET );
    char name[] = "2O\000";
    RegisterAs(name);
    uart_server server;
    uart_server_initial(&server,MyParentTid());
    server.mes_send->type = UART_UART2_OUT;
    while(1){
        AwaitEvent(UART2_OUT);
        Send(3,server.buf_send,server.mes_size,\
            server.buf_rpy,server.mes_size);
        *data = server.mes_rpy->buffer;
    }
}

//-----------------------put & get------------------------------
void uart_server_put(uart_server *server, char c, UART_type type) {
    server->mes_send->type = type;
    server->mes_send->buffer = c;
    Send(server->server_tid,server->buf_send,server->mes_size,server->buf_rpy,server->mes_size);
}

void uart_server_puts(uart_server *server, char *s, UART_type type) {
    server->mes_send->type = type;
 //   server->mes_send->str = s;
    Send(server->server_tid, server->buf_send, server->mes_size, server->buf_rpy, server->mes_size);
}

char uart_server_get(uart_server *server, UART_type type) {
    server->mes_send->type = type;
    Send(server->server_tid,server->buf_send,server->mes_size,server->buf_rpy,server->mes_size);
    return server->mes_rpy->buffer;
}
