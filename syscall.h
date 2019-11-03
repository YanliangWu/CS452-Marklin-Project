#ifndef _SYSCALL_H_
#define _SYSCALL_H_


// these definition are located in asm.s.
// include to avoid complier error.
void asm_boot();

void asm_create(int priority, void (*code)());

void asm_mytid();

void asm_myparenttid();

void asm_pass();

void asm_exit();

void asm_receive();

void asm_send();

void asm_reply();

void asm_awaitevent();

void asm_getIdle();

// These function are in syscall.c
void Boot(void *code);

int Create( int priority, void (*code)());

int MyTid();

int MyParentTid();

void Pass();

void Exit();

int Send( int tid, char *msg, int msglen, char *reply, int replylen );

int Receive( int *tid, char *msg, int msglen );

int Reply(int tid, char *reply, int replylen );

int AwaitEvent(int eventid);

int getIdle();

void Terminate();

int getTrackID();

#endif


