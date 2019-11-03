#ifndef UART_H_
#define UART_H_

#define U1TR_BUF_SIZE 1000
#define U2TR_BUF_SIZE 10000
#define U1RC_BUF_SIZE 200
#define U2RC_BUF_SIZE 100
#define COM_BUF_SIZE 100
#define MAX_DEBUG_SIZE 3

#include "kernel_swi.h"

typedef enum UART_type {
    UART_CLOCK     = 1,
    UART_UART1_IN  = 2,
    UART_UART1_OUT = 3,
    UART_UART2_IN  = 4,
    UART_UART2_OUT = 5,
    UART_PUT_UART1 = 6,
    UART_PUT_UART2 = 7,
    UART_GET_UART1 = 8,
    UART_GET_UART2 = 9,
    UART_PUT_UART1_STR = 10,
    UART_PUT_UART2_STR = 11
} UART_type;

typedef struct UART_message {
    UART_type type;
    char buffer;
    char str[50];
    int len;
} UART_message;

typedef struct BufferDescription{
    int count;
    int start;
    int end;
    int size;
    char *Buffer;
} BD;

typedef struct Debug{
    short index;
    short size;
    short row;
    short col;

} uart_debug;

typedef struct uart_server{
    char buf_send[sizeof(UART_message)];
    char buf_rcv[sizeof(UART_message)];
    char buf_rpy[sizeof(UART_message)];
    UART_message * mes_send;
    UART_message * mes_rcv;
    UART_message * mes_rpy;
    uart_debug debug[MAX_DEBUG_SIZE];
    int mes_size;
    int server_tid;
    int client_tid;
} uart_server;


void buffer_init(BD * buf, char *str, int size);
void buffer_put(BD *buf, char c);
char buffer_get(BD *buf);

void uart_server_boot();
void uart_server_initial(uart_server *server, int tid);
void uart_server_print_command(uart_server *server, BD *com, BD *buf,short *Uart2NotifierReady,int uart2out);
//----------------------------uart server case handle--------------------------

void uart1_input_notifier_boot();
void uart1_output_notifier_boot();
void uart2_input_notifier_boot();
void uart2_output_notifier_boot();

void uart_server_put(uart_server *server, char c, UART_type type);
void uart_server_puts(uart_server *server, char *s, UART_type type);
char uart_server_get(uart_server *server, UART_type type);


#endif
