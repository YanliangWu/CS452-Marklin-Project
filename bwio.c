/*
 * bwio.c - busy-wait I/O routines for diagnosis
 *
 * Specific to the TS-7200 ARM evaluation board
 *
 */

#include "bwio.h"
#include "ts7200.h"
#include "syscall.h"
#include "string.h"
#include "uartserver.h"
/*
 * The UARTs are initialized by RedBoot to the following state
 * 	115,200 bps
 * 	8 bits
 * 	no parity
 * 	fifos enabled
 */



/////////////////////////////////////////////////////////////////
int bwsetfifo( int channel, int state ) {
	int *line, buf;
	switch( channel ) {
		case COM1:
			line = (int *)( UART1_BASE + UART_LCRH_OFFSET );
			//buf = *line;
			//buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
			//buf = (buf | STP2_MASK);
			//buf = (((buf | STP2_MASK) | WLEN_MASK) & (~PEN_MASK)) & (~BRK_MASK);
			break;
		case COM2:
			line = (int *)( UART2_BASE + UART_LCRH_OFFSET );
			//buf = *line;
			//buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
			break;
		default:
			return -1;
			break;
	}
	buf = *line;
	buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
	*line = buf;
	return 0;
}


int bwsetspeed( int channel, int speed ) {
	int *high, *low;
	switch( channel ) {
		case COM1:
			high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
			low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
			break;
		case COM2:
			high = (int *)( UART2_BASE + UART_LCRM_OFFSET );
			low = (int *)( UART2_BASE + UART_LCRL_OFFSET );
			break;
		default:
			return -1;
			break;
	}
	switch( speed ) {
		case 115200:
			*high = 0x0;
			*low = 0x3;
			return 0;
		case 2400:
			*high = 0x0;
			*low = 191;
			return 0;
		default:
			return -1;
	}
}

int bwputc( int channel, char c ) {
	int *flags, *data;
	switch( channel ) {
		case COM1:
			flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
			data = (int *)( UART1_BASE + UART_DATA_OFFSET );
			break;
		case COM2:
			flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
			data = (int *)( UART2_BASE + UART_DATA_OFFSET );
			break;
		default:
			return -1;
			break;
	}
	while(!(((channel == COM1) && (*flags & CTS_MASK) && (*flags & TXFE_MASK)) || ((channel == COM2) && (*flags & TXFE_MASK)))) ;
	*data = c;
	return 0;
}

char c2x( char ch ) {
	if ( (ch <= 9) ) return '0' + ch;
	return 'a' + ch - 10;
}

int bwputx( int channel, char c ) {
	char chh, chl;

	chh = c2x( c / 16 );
	chl = c2x( c % 16 );
	bwputc( channel, chh );
	return bwputc( channel, chl );
}

int bwputr( int channel, unsigned int reg ) {
	int byte;
	char *ch = (char *) &reg;

	for( byte = 3; byte >= 0; byte-- ) bwputx( channel, ch[byte] );
	return bwputc( channel, ' ' );
}

int bwputstr( int channel, char *str ) {
	while( *str ) {
		if( bwputc( channel, *str ) < 0 ) return -1;
		str++;
	}
	return 0;
}

void bwputw( int channel, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) bwputc( channel, fc );
	while( ( ch = *bf++ ) ) bwputc( channel, ch );
}

int bwgetc( int channel ) {
	int *flags, *data;
	unsigned char c;

	switch( channel ) {
		case COM1:
			flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
			data = (int *)( UART1_BASE + UART_DATA_OFFSET );
			break;
		case COM2:
			flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
			data = (int *)( UART2_BASE + UART_DATA_OFFSET );
			break;
		default:
			return -1;
			break;
	}
	while ( !( *flags & RXFF_MASK ) ) ;
	c = *data;
	return c;
}


int bwa2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

char bwa2i( char ch, char **src, int base, int *nump ) {
	int num, digit;
	char *p;

	p = *src; num = 0;
	while( ( digit = bwa2d( ch ) ) >= 0 ) {
		if ( digit > base ) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p; *nump = num;
	return ch;
}

void bwui2a( unsigned int num, unsigned int base, char *bf ) {
	int n = 0;
	int dgt;
	unsigned int d = 1;

	while( (num / d) >= base ) d *= base;
	while( d != 0 ) {
		dgt = num / d;
		num %= d;
		d /= base;
		if( n || dgt > 0 || d == 0 ) {
			*bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
			++n;
		}
	}
	*bf = 0;
}

void bwi2a( int num, char *bf ) {
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	bwui2a( num, 10, bf );
}

void bwformat ( int channel, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;


	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			bwputc( channel, ch );
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch ( ch ) {
				case '0':
					lz = 1; ch = *(fmt++);
					break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					ch = bwa2i( ch, &fmt, 10, &w );
					break;
			}
			switch( ch ) {
				case 0: return;
				case 'c':
						bwputc( channel, va_arg( va, char ) );
						break;
				case 's':
						bwputw( channel, w, 0, va_arg( va, char* ) );
						break;
				case 'u':
						bwui2a( va_arg( va, unsigned int ), 10, bf );
						bwputw( channel, w, lz, bf );
						break;
				case 'd':
						bwi2a( va_arg( va, int ), bf );
						bwputw( channel, w, lz, bf );
						break;
				case 'x':
						bwui2a( va_arg( va, unsigned int ), 16, bf );
						bwputw( channel, w, lz, bf );
						break;
				case '%':
						bwputc( channel, ch );
						break;
			}
		}
	}
}

void bwprintf( int channel, char *fmt, ... ) {
	va_list va;
	va_start(va,fmt);
	bwformat( channel, fmt, va );
	va_end(va);
}

//-------------------Serial I/O-----------------------

int kerputc(BD *BD, char c){
	BD->Buffer[BD->end] = c;
	BD->end = (BD->end+1)%BD->size;
	BD->count ++;
	return 0;
}

//-------kernel io(part2)------------
int kerputx( BD *BD, char c ) {
	char chh, chl;

	chh = c2x( c / 16 );
	chl = c2x( c % 16 );
	kerputc( BD, chh );
	return kerputc( BD, chl );
}

int kerputr( BD *BD, unsigned int reg ) {
	int byte;
	char *ch = (char *) &reg;

	for( byte = 3; byte >= 0; byte-- ) kerputx( BD, ch[byte] );
	return kerputc( BD, ' ' );
}


void kerputw( BD *BD, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) kerputc( BD, fc );
	while( ( ch = *bf++ ) ) kerputc( BD, ch );
}

void kerformat ( BD *BD, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;


	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			kerputc( BD, ch );
			//bwputc(chan->name,ch);
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch ( ch ) {
				case '0':
					lz = 1; ch = *(fmt++);
					break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					ch = bwa2i( ch, &fmt, 10, &w );
					break;
			}
			switch( ch ) {
				case 0: return;
				case 'c':
					kerputc( BD, va_arg( va, char ) );
					//bwputc( chan->name, va_arg( va, char ) );
					break;
				case 's':
					kerputw( BD, w, 0, va_arg( va, char* ) );
					//bwputw( chan->name, w, 0, va_arg( va, char* ) );
					break;
				case 'u':
					bwui2a( va_arg( va, unsigned int ), 10, bf );
					kerputw( BD, w, lz, bf );
					//bwputw( chan->name, w, lz, bf );
					break;
				case 'd':
					bwi2a( va_arg( va, int ), bf );
					kerputw( BD, w, lz, bf );
					//bwputw( chan->name, w, lz, bf );
					break;
				case 'x':
					bwui2a( va_arg( va, unsigned int ), 16, bf );
					kerputw( BD, w, lz, bf );
					//bwputw( chan->name, w, lz, bf );
					break;
				case '%':
					kerputc( BD, ch );
					//bwputc( chan->name, ch );
					break;
			}
		}
	}
}

void kerprintf( int COM, uart_server *server, char *fmt, ...) {
	BD buf;
	buffer_init(&buf, server->mes_send->str,40);
	va_list va;
	va_start(va,fmt);
	kerformat( &buf, fmt, va );
	va_end(va);

	switch(COM){
		case COM1:{
			server->mes_send->type = UART_PUT_UART1_STR;
			server->mes_send->len = buf.count;
			Send(3, server->buf_send, sizeof(UART_message), server->buf_rpy,
				 sizeof(UART_message));
			int i;
			for(i=0;i<40;i++)
				server->mes_send->str[i]='\000';
		}
		case COM2:{
			server->mes_send->type = UART_PUT_UART2_STR;
			server->mes_send->len = buf.count;
			Send(3, server->buf_send, sizeof(UART_message), server->buf_rpy,
				 sizeof(UART_message));
			int i;
			for(i=0;i<40;i++)
				server->mes_send->str[i]='\000';
		}
	}
}


void debug( uart_server *server, int k, char *fmt, ...) {

	server->mes_send->str[0]='\033';
	server->mes_send->str[1]='[';
	server->mes_send->str[2]=(server->debug[k].row+server->debug[k].index)/10+'0';
	server->mes_send->str[3]=(server->debug[k].row+server->debug[k].index)%10+'0';
	server->mes_send->str[4]=';';
	server->mes_send->str[5]=server->debug[k].col/10+'0';
	server->mes_send->str[6]=server->debug[k].col%10+'0';
	server->mes_send->str[7]='H';

	BD buf;
	buffer_init(&buf, &server->mes_send->str[8],40);
	va_list va;
	va_start(va,fmt);
	kerformat( &buf, fmt, va );
	va_end(va);
	int i;

	server->mes_send->type = UART_PUT_UART2_STR;
	server->mes_send->len = buf.count;
	Send(3, server->buf_send, server->mes_size, server->buf_rpy,
		 server->mes_size);
	for(i=0;i<50;i++)
		server->mes_send->str[i]='\000';
	server->debug[k].index=(server->debug[k].index+1)%server->debug[k].size;

}

