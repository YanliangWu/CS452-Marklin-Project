#include "task.h"
#include "bwio.h"

int assign_tid(TD * arr, int size){
	int i = 0;
	for (i = 1; i < size; i++){
		if (arr[i].task_state == EMPTY){
			return i;	
		}
	}	
	return 0;	
}


void TD_initial(TD * arr, int size){
    int i;
    for(i=1;i<size;i++){
        arr[i].task_state = EMPTY;
    }
}

int TD_create (TD *arr, void *pc, int task_parent_tid, int priority, priority_queue *q){
	unsigned int tid = assign_tid(arr, MAX_TASK_NUM);
    //bwprintf(COM2, "TD_create-> pc: %x \n\r",(int *)pc);
	arr[tid].task_tid = tid;
	arr[tid].task_sp = (void*)(USER_STACK_BASE - (tid * USER_STACK_SIZE));
	arr[tid].task_pc = pc;
    arr[tid].task_cpsr = 16;
    arr[tid].task_return_value = 0;
	arr[tid].task_parent_tid = task_parent_tid; 
	//bwprintf(COM2,"Setting parent tid to: %d \n\r",task_parent_tid);
	arr[tid].task_priority = priority;
	arr[tid].task_state = READY;
	arr[tid].wl_start = 0;
	arr[tid].wl_end = 0;
	*(arr[tid].task_sp - 13) = (unsigned int)pc;
	*((int *)(arr[tid].task_sp - 2)) = (unsigned int)(arr[tid].task_sp);
	arr[tid].task_sp -= 13;
    priority_queue_push(q,priority,tid);
//	bwprintf(COM2, "TD_create finished->tid: %d, priority: %d \n\r",tid,priority);
	return tid;
}

void TD_save (TD * arr,  void * task_sp, void * task_pc,int task_tid,int task_rv, int task_cpsr, priority_queue *q){
	arr[task_tid].task_sp = task_sp; 
	arr[task_tid].task_pc = task_pc;
    arr[task_tid].task_cpsr = task_cpsr;
	arr[task_tid].task_return_value = task_rv;
//	bwprintf(COM2, "------------syscall save sp:%x |pc:%x |cpsr:%x \n\r", task_sp, task_pc, 16);
    priority_queue_push(q,arr[task_tid].task_priority,task_tid);
}

// function who claled td_block will not be schedule again until unblocked
void TD_block_save(TD * arr,  void * task_sp, void * task_pc,int task_tid, int task_rv, int task_cpsr, priority_queue *q){
	arr[task_tid].task_sp = task_sp;
	arr[task_tid].task_pc = task_pc;
	arr[task_tid].task_cpsr = task_cpsr,
	arr[task_tid].task_return_value = task_rv;
//	bwprintf(COM2, "-------------block save sp:%x |pc:%x |cpsr:%x \n\r", task_sp, task_pc, 16);
	arr[task_tid].task_state = BLOCKED;
}

// unblock is basically just push that back to ready queue
void TD_unblock(TD * arr,int task_tid, priority_queue *q){
	arr[task_tid].task_state = READY;
	priority_queue_push(q,arr[task_tid].task_priority,task_tid);
}

int TD_next (priority_queue *q){
	return priority_queue_pop(q);
}


