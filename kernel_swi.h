#ifndef KERNEL_H_
#define KERNEL_H_

#include "task.h"
#include "global.h"
#define TIMER_IRQ_STACK_START 0x01D00000
//#define DEBUG_MODE 1
#define DEBUG_SYSCALL 0
//#define DEBUG_SYSCALL 0
#define KERNEL_STACK_BASE 0x01F00000
#define KERNEL_STACK_SIZE 0x00300000

typedef struct msgDescriptor{
	char * msg;
	int msglen;
	char *rep;
	int replen;
	int orig_tid;
	int dest_tid;
}MD;

typedef enum EVENT{
	CLOCK = 1,
    UART1_IN = 2,
    UART1_OUT = 3,
    UART2_IN = 4,
    UART2_OUT = 5,
}EVENT;

typedef struct KD {
	void *user_sp;
	void *user_pc;
	int user_cpsr;
	int user_return_value;

	TD tasks_td[MAX_TASK_NUM + 1];
	priority_queue tasks_queue;
	int tasks_count;
	void *redboot_sp;
	void *redboot_pc;
	int redboot_cpsr;
	int current_tid;
	int event2td[5];
	int init_flag;
    int time;
    int idle_time;
	// clear to send and buffer empty
	int UART1_C2S;
	int UART1_TBE;
	int track_id;
}KD;

void k_Boot(KD *k_state);
void k_Create(KD *k_state);
void k_MyTid(KD *k_state);
void k_MyParentTid(KD *k_state);
void k_Pass(KD *k_state);
void k_Exit(KD *k_state);
void k_Send(KD *k_state);
void k_Receive(KD *k_state);
void k_Reply(KD *k_state);
void k_awaitEvent(KD* k_state);
void k_getIdle(KD* k_state);
void k_getTrackID(KD *k_state);
#endif
