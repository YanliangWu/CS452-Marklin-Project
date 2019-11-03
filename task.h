#ifndef _TASK_H_
#define _TASK_H_

#include "priority_queue.h"

#define USER_STACK_BASE   0x01B00000
//#define USER_STACK_SIZE   0x0007A000
#define USER_STACK_SIZE   0x000B0000

typedef enum taskstate{
	ACTIVE = 1,
	READY = 2,
	BLOCKED = 3,
	ZOMBIE = 4,
	EMPTY = 5,
	SEND_BLOCKED = 6,
	RECE_BLOCKED = 7,
	REPL_BLOCKED = 8
} taskstate;

typedef struct TD{
	unsigned int *task_sp;
	unsigned int *task_pc;
	int task_cpsr;
	int task_return_value;

	unsigned int task_tid;
	int task_parent_tid;
	taskstate task_state;
	int task_priority;
	char * send_msg;
	char * rec_msg;
	char * rep_msg;
	int rec_len;
	int send_len;
	int rep_len;
	int orig_tid;
	int * rec_tid;
	int rece_wl [20];
	int wl_start;
	int wl_end;
}TD;

void TD_initial (TD * arr, int size);

int TD_create (TD *arr, void *pc, int task_parent_tid, int priority, priority_queue *q);

void TD_save (TD * arr,  void * task_sp, void * task_pc,int task_tid,int task_rv, int task_cpsr,priority_queue *q);

void TD_exit (TD * arr, int tid);

int TD_next (priority_queue *q);

void TD_block_save(TD * arr,  void * task_sp, void * task_pc,int task_tid,int task_rv, int task_cpsr, priority_queue *q);

void TD_unblock(TD * arr,int task_tid, priority_queue *q);
#endif
