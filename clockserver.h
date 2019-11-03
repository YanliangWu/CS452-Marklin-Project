#include "bwio.h"
#include "kernel_swi.h"
#include "kernel_hwi.h"
#include "syscall.h"
#include "nameserver.h"
#include "print.h"

typedef enum CL_type {
    CL_NOTIFIER = 1,
    CL_TIME_REQUEST = 2,
    CL_DELAY_REQUEST = 3,
} CL_type;

typedef struct CL_message {
    CL_type type;
    int num;
    int delay_time;
    int delay_num;
} CL_message;

//----------------------------------server------------------------------
typedef struct CL_server{
    int tick;
    int task_delay[50];
    char buf_send[sizeof(CL_message)];
    char buf_rcv[sizeof(CL_message)];
    char buf_rpy[sizeof(CL_message)];
    CL_message * mes_send;
    CL_message * mes_rcv;
    CL_message * mes_rpy;
    int client_tid;
    int server_tid;
    int mes_size;
} clock_server;

void clock_server_boot();
void clock_server_initial(clock_server *server, int server_tid);

void clock_server_notifier_handle(clock_server *server);
void clock_server_delay_handle(clock_server *server);
void clock_server_time_handle(clock_server *server);

void clock_server_unblock(clock_server *server);
void clock_server_settime();

//---------------------------------notifier----------------------------------
void clock_notifier_boot();

//---------------------------------idle----------------------------------
void clock_idle_boot ();

//-----------------------------------application----------------------------
void Delay(clock_server *server, int tick);

