#include <ts7200.h>
#include "initialization.h"
#include "bwio.h"
#include "syscall.h"
#include "nameserver.h"
#include "rps.h"
#include "kernel_hwi.h"
#include "clockserver.h"
#include "uartserver.h"
#include "print.h"

void Channel_SetFifo( int COM, int state ) {
    int *line, buf;
    switch( COM ) {
        case COM1:
            line = (int *)( UART1_BASE + UART_LCRH_OFFSET );
            buf = *line;
            buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
            //  2 stop bits, 8 bit words, no parity, no beak
            buf = (((buf | STP2_MASK) | WLEN_MASK) & (~PEN_MASK)) & (~BRK_MASK);
            break;
        case COM2:
            line = (int *)( UART2_BASE + UART_LCRH_OFFSET );
            buf = *line;
            buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
            break;
        default:
            return;
    }

    *line = buf;

}


void set_channel(channel* com1, channel* com2){
   // bwsetfifo(COM2,OFF);
   // bwsetfifo(COM1,OFF);
    bwsetspeed(COM2, 115200);
    bwsetspeed(COM1, 2400);
    Channel_SetFifo( COM1, OFF);
    Channel_SetFifo( COM2, OFF);
	//int *line = (int *)( UART1_BASE + UART_LCRH_OFFSET );
    //*line = (((*line | STP2_MASK) | WLEN_MASK) & (~PEN_MASK)) & (~BRK_MASK);
        
    com1->name = COM1;
    com1->input_reader=0;
    com1->input_writer=0;
    com1->output_reader=0;
    com1->output_writer=0;
    com2->name = COM2;
    com2->input_reader=0;
    com2->input_writer=0;
    com2->output_reader=0;
    com2->output_writer=0;

    bwprintf(COM2, "\n\r\n\r\n\r");
}

