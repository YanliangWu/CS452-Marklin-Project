#ifndef _INIT_H_
#define _INIT_H_
#include "bwio.h"

void set_channel(channel* com1, channel* com2);

void first_kernel_task();

void create_test();

void sender1();
void sender2();

void receiver();

void nameserver_test1();

void nameserver_test2();

#endif
