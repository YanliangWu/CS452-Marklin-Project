#ifndef BWIO_H_
#define BWIO_H_

#include "uartserver.h"
/*
 * bwio.h
 */

typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

#define COM1	0
#define COM2	1

#define ON	1
#define	OFF	0

//kernel io//////////////////////



//polling loop io/////////////////
typedef struct channel{
    int name;
    char input_buffer[10000];
    int input_reader;
    int input_writer;
    char output_buffer[10000];
    int output_reader;
    int output_writer;
}channel;

void kerprintf( int COM, uart_server *server, char *fmt, ...);


void debug( uart_server *server, int i, char *fmt, ...);

int putstr(int port, char *c, int len ,uart_server * server);

int bwsetfifo( int channel, int state );

int bwsetspeed( int channel, int speed );

int bwputc( int channel, char c );

int bwgetc( int channel );

int bwputx( int channel, char c );

int bwputstr( int channel, char *str );

int bwputr( int channel, unsigned int reg );

void bwputw( int channel, int n, char fc, char *bf );

void bwprintf( int channel, char *format, ... );
#endif
