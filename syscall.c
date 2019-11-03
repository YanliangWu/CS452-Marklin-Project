#include "bwio.h"
#include "syscall.h"
#include "global.h"


int Create( int priority, void (*code) ( ) ) {
	register int retval_reg asm("r0");
	int retval;
	asm volatile(
	"stmfd sp!, {r0, r1}\n\t"
			"swi %1\n\t"
			"mov %0, r0\n\t"
			"ldmfd sp!, {r1, r2}\n\t"
	: "+r"(retval_reg)
	: "i"(SYSCALL_CREATE)
	);
	retval = retval_reg;
	return retval;
}

int Send( int tid, char *msg, int msglen, char *reply, int replylen ) {
	register int retval_reg asm("r0");
	int retval;
	asm volatile(
	/* store arguments in ascending order regarding the stack */
	"stmfd sp!, {r0}\n\t"            // tid
			"ldr r0, [sp, #4]\n\t"
			"stmfd sp!, {r0}\n\t"            //replylen
			"stmfd sp!, {r1, r2, r3}\n\t"   // reply, msglen, msg
			"swi %1\n\t"
			"mov %0, r0\n\t"
			"add sp, sp, #20\n\t"
	: "+r"(retval_reg)
	: "i"(SYSCALL_SEND)
	);

	retval = retval_reg;

	return retval;
}

int Receive( int *tid, char *msg, int msglen ) {
	register int retval_reg asm("r0");
	int retval;
	asm volatile(
	"stmfd sp!, {r0, r1, r2}\n\t"
			"swi %1\n\t"
			"mov %0, r0\n\t"
			"add sp, sp, #12\n\t"
	: "+r"(retval_reg)
	: "i"(SYSCALL_RECEIVE)
	);
	retval = retval_reg;
	return retval;
}

int Reply(int tid, char *reply, int replylen ) {
	register int retval_reg asm("r0");
	int retval;
	asm volatile(
	"stmfd sp!, {r0, r1, r2}\n\t"
			"swi %1\n\t"
			"mov %0, r0\n\t"
			"add sp, sp, #12\n\t"
	: "+r"(retval_reg)
	: "i"(SYSCALL_REPLY)
	);
	retval = retval_reg;
	return retval;
}

int MyTid( ) {
	register int retval_reg asm("r0");
	int retval;
	asm volatile(
	"swi %1\n\t"
			"mov %0, r0\n\t"
	: "+r"(retval_reg)
	: "i"(SYSCALL_MYTID)
	);
	retval = retval_reg;
	return retval;
}

int MyParentTid( ) {
	register int retval_reg asm("r0");
	int retval;
	asm volatile(
	"swi %1\n\t"
			"mov %0, r0\n\t"
	: "+r"(retval_reg)
	: "i"(SYSCALL_MYPARENTTID)
	);
	retval = retval_reg;
	return retval;
}

void Pass() {
	asm volatile(
	"swi %0\n\t"
	:: "i"(SYSCALL_PASS)
	);
}

void Exit() {
	asm volatile(
	"swi %0\n\t"
	:: "i"(SYSCALL_EXIT)
	);
}

int AwaitEvent( int eventid ) {
	register int retval_reg asm("r0");
	int retval;
	asm volatile(
	"stmfd sp!, {r0}\n\t"
			"swi %1\n\t"
			"mov %0, r0\n\t"
			"add sp, sp, #4\n\t"
	: "+r"(retval_reg)
	: "i"(SYSCALL_AWAITEVENT)
	);
	retval = retval_reg;
	return retval;
}


int getIdle() {
	register int retval_reg asm("r0");
	int retval;
	asm volatile(
	"swi %1\n\t"
			"mov %0, r0\n\t"
	: "+r"(retval_reg)
	: "i"(SYSCALL_GETIDLE)
	);
	retval = retval_reg;
	return retval;
}


void Terminate() {
	register int retval_reg asm("r0");
	int retval;
	asm volatile(
	"swi %1\n\t"
			"mov %0, r0\n\t"
	: "+r"(retval_reg)
	: "i"(SYSCALL_TERMINATE)
	);
}


int getTrackID() {
	register int retval_reg asm("r0");
	int retval;
	asm volatile(
	"swi %1\n\t"
			"mov %0, r0\n\t"
	: "+r"(retval_reg)
	: "i"(SYSCALL_GETTRACKID)
	);
	retval = retval_reg;
	return retval;
}

