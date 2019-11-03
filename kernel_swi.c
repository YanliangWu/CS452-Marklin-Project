#include "kernel_swi.h"
#include "bwio.h"
#include "print.h"
#include "string.h"
#include "ts7200.h"
#include "initialization.h"
#include "syscall.h"
#include "nameserver.h"
#include "kernel_hwi.h"
#include "clockserver.h"
#include "trainserver.h"
#include "uartserver.h"


void asm_k_start();
void asm_k_exit();
void asm_getIdle();



void k_Create(KD *k_state) {
    //bwprintf(COM2, "k_Create: Current tid is: %d \n\r",k_state->current_tid);
    register unsigned int priority_reg asm("r0");
    unsigned int priority;
    register void (*code_reg)() asm("r1");
    void (*code)();
    register unsigned int *cur_sp_reg asm("r3") = (k_state->tasks_td[k_state->current_tid]).task_sp;

    asm volatile(
    "msr cpsr_c, #0xdf\n\t"
            "add r3, %2, #56\n\t" // 14 registers + pc + retval saved
            "ldmfd r3, {%0, %1}\n\t"
            "msr cpsr_c, #0xd3\n\t"
    : "+r"(priority_reg), "+r"(code_reg), "+r"(cur_sp_reg)
    );
    priority = priority_reg;
    code = code_reg;

    if(priority < 1 && priority > 32){
        k_state->tasks_td[k_state->current_tid].task_return_value = -1;
    }
    else {
        int curr_tid = k_state->current_tid;
        int return_tid = TD_create(k_state->tasks_td, code, curr_tid, priority, &(k_state->tasks_queue));
        //if (DEBUG_SYSCALL)
        //bwprintf(COM2, "SYSCALL: TASK %d ( Create %d )\n\r", k_state->current_tid, return_tid);

        k_state->tasks_count++;
        k_state->tasks_td[k_state->current_tid].task_return_value = return_tid;
        TD_unblock(k_state->tasks_td, k_state->current_tid, &(k_state->tasks_queue));
    }
}


void k_MyTid(KD *k_state){
    k_state->tasks_td[k_state->current_tid].task_return_value = k_state->current_tid;
    TD_unblock(k_state->tasks_td, k_state->current_tid, &(k_state->tasks_queue));
    if(DEBUG_SYSCALL)
        bwprintf(COM2,"SYSCALL: TASK %d ( MyTid )\n\r", k_state->current_tid);
}


void k_MyParentTid(KD *k_state) {
    int parent_tid = k_state->tasks_td[k_state->current_tid].task_parent_tid;
    k_state->tasks_td[k_state->current_tid].task_return_value = parent_tid;
    TD_unblock(k_state->tasks_td, k_state->current_tid, &(k_state->tasks_queue));
    if(DEBUG_SYSCALL)
        bwprintf(COM2,"SYSCALL: TASK %d ( MyParentTid %d)\n\r", k_state->current_tid,parent_tid);
}


void k_Pass(KD *k_state) {
    if(DEBUG_SYSCALL)
        bwprintf(COM2,"SYSCALL: TASK %d ( Pass )\n\r", k_state->current_tid);
    TD_unblock(k_state->tasks_td, k_state->current_tid, &(k_state->tasks_queue));
}


void k_Exit(KD *k_state) {
    if(DEBUG_SYSCALL)
        bwprintf(COM2,"SYSCALL: TASK %d ( Exit )\n\r", k_state->current_tid);
    k_state->tasks_td[k_state->current_tid].task_state = ZOMBIE;
    k_state->tasks_count --;
}



unsigned int get_message_tid(unsigned int *sp, int offset) {
//    debug("sp, *sp: %x, %x", sp, *sp);
//    debug("offset: %d", offset);
    register unsigned int  int_reg     asm("r1");
    register int           offset_reg  asm("r2") = offset;
    register unsigned int  *cur_sp_reg asm("r3") = sp;
    int rtn;
    /* get the first argument: tid */
    asm volatile(
    "msr cpsr_c, #0xdf\n\t"       // switch to system mode
            "add r3, %1, %2\n\t"          // 10 registers + pc saved
            "ldmfd r3!, {%0}\n\t"         // load tid arg
            "msr cpsr_c, #0xd3\n\t"       // switch to svc mode
    : "+r"(int_reg), "+r"(cur_sp_reg)
    : "r"(offset_reg)
    );
    rtn = int_reg;
    return rtn;
}

char *get_message( unsigned int *sp, int offset, int *msglen ) {
    /* get the next two arguments: *msg, and msglen */
    register char          *char_reg   asm("r0");
    register unsigned int  int_reg     asm("r1");
    register int           offset_reg  asm("r2") = offset;
    register unsigned int  *cur_sp_reg asm("r3") = sp;
    char *msg;
    asm volatile(
    "msr cpsr_c, #0xdf\n\t"       // switch to system mode
            "add r3, %2, %3\n\t"          // 10 registers + pc saved
            "ldmfd r3!, {%0, %1}\n\t"     // store two args
            "msr cpsr_c, #0xd3\n\t"       // switch to svc mode
    : "+r"(char_reg), "+r"(int_reg), "+r"(cur_sp_reg)
    : "r"(offset_reg)
    );
    *msglen = int_reg;
    msg = char_reg;
    return msg;
}

void k_Send(KD *k_state) {

    unsigned int tid;
    int msglen, replylen;
    char *msg, *reply;

    tid = get_message_tid(k_state->tasks_td[k_state->current_tid].task_sp, 72);
    // msglen_s set
    msg = get_message( k_state->tasks_td[k_state->current_tid].task_sp, 56, &msglen );
    // replylen_s set
    reply = get_message(  k_state->tasks_td[k_state->current_tid].task_sp, 64, &replylen );


    taskstate valid = k_state->tasks_td[tid].task_state;
    if(valid != ZOMBIE && valid != EMPTY){
        if(valid != SEND_BLOCKED){
            k_state->tasks_td[k_state->current_tid].send_msg = msg;
            k_state->tasks_td[k_state->current_tid].send_len = msglen;
            k_state->tasks_td[k_state->current_tid].rep_msg = reply;
            k_state->tasks_td[k_state->current_tid].rep_len = msglen;
            k_state->tasks_td[tid].rece_wl[k_state->tasks_td[tid].wl_end] = k_state->current_tid;
            k_state->tasks_td[tid].wl_end = (k_state->tasks_td[tid].wl_end + 1) % 20;
            k_state->tasks_td[k_state->current_tid].task_state = RECE_BLOCKED;
            k_state->tasks_td[k_state->current_tid].task_return_value = msglen;
        }
        else {
            k_state->tasks_td[k_state->current_tid].rep_msg = reply;
            k_state->tasks_td[k_state->current_tid].rep_len = msglen;
            stringcpy(k_state->tasks_td[tid].rec_msg, msg, msglen);
            k_state->tasks_td[tid].rec_len = msglen;
            *(k_state->tasks_td[tid].rec_tid) = k_state->current_tid;
            k_state->tasks_td[tid].orig_tid = k_state->current_tid;
            k_state->tasks_td[k_state->current_tid].task_state = REPL_BLOCKED;
            TD_unblock(k_state->tasks_td,tid,&(k_state->tasks_queue));
            k_state->tasks_td[k_state->current_tid].task_return_value = msglen;
        }
    }
    else{
        if(tid > MAX_TASK_NUM){
            k_state->tasks_td[k_state->current_tid].task_return_value = -1;

        }
        else if(valid == ZOMBIE || valid == EMPTY){
            k_state->tasks_td[k_state->current_tid].task_return_value = -3;
        }
    }
}


void k_Receive(KD *k_state) {

    unsigned int *ptid_r;
    int msglen_r;
    char *msg_r;

    /* get receiver's arguments: *tid, *msg and msglen */
    ptid_r = (unsigned int *) get_message_tid( k_state->tasks_td[k_state->current_tid].task_sp, 56 );
    msg_r = get_message(k_state->tasks_td[k_state->current_tid].task_sp, 60, &msglen_r );

    int start = k_state->tasks_td[k_state->current_tid].wl_start;
    int end = k_state->tasks_td[k_state->current_tid].wl_end;
    int index = k_state->tasks_td[k_state->current_tid].rece_wl[start];

    if(start != end){
        k_state->tasks_td[k_state->current_tid].wl_start = (k_state->tasks_td[k_state->current_tid].wl_start + 1) % 20;
        taskstate valid = k_state->tasks_td[index].task_state;
        if(valid == RECE_BLOCKED){
            stringcpy(msg_r, k_state->tasks_td[index].send_msg,  k_state->tasks_td[index].send_len);
            *ptid_r = index;
            k_state->tasks_td[index].task_state = REPL_BLOCKED;
            // unblock myself to let it go
            TD_unblock(k_state->tasks_td, k_state->current_tid, &(k_state->tasks_queue));
            if(DEBUG_SYSCALL)
                bwprintf(COM2,"SYSCALL: TASK %d ( Receive from %d )\n\r", k_state->current_tid, *ptid_r);
        }
        else{
            k_state->user_return_value = -3;
        }
    }
    else{
        // block itself
        k_state->tasks_td[k_state->current_tid].rec_msg = msg_r;
        k_state->tasks_td[k_state->current_tid].rec_len = k_state;
        k_state->tasks_td[k_state->current_tid].rec_tid = ptid_r;
        k_state->tasks_td[k_state->current_tid].task_state = SEND_BLOCKED;
        if(DEBUG_SYSCALL)
            bwprintf(COM2,"SYSCALL: TASK %d ( Receive Block ), (%d)\n\r", k_state->current_tid, SEND_BLOCKED);
    }
}

void k_Reply(KD *k_state) {

    unsigned int target_tid;
    int replylen_r;
    //int replylen_s;
    char *reply_r;
    //char *reply_s;

    /* get replyer's arg: target tid */
    target_tid = get_message_tid( k_state->tasks_td[k_state->current_tid].task_sp, 56 );
    reply_r = get_message( k_state->tasks_td[k_state->current_tid].task_sp, 60, &replylen_r );
	if(DEBUG_SYSCALL)
		bwprintf(COM2,"SYSCALL: TASK %d ( Reply to %d )\n\r", k_state->current_tid,target_tid);
	stringcpy(k_state->tasks_td[target_tid].rep_msg, reply_r, replylen_r);
	TD_unblock(k_state->tasks_td, target_tid, &(k_state->tasks_queue));
    TD_unblock(k_state->tasks_td, k_state->current_tid, &(k_state->tasks_queue));
}

void k_awaitEvent(KD *k_state) {
    register int event_type_reg asm("r0");
    int event_type;
    register unsigned int *cur_sp_reg asm("r3") = (k_state->tasks_td[k_state->current_tid]).task_sp;
    asm volatile(
    "msr cpsr_c, #0xdf\n\t"
            "add r3, %1, #56\n\t"
            "ldmfd r3, {%0}\n\t"
            "msr cpsr_c, #0xd3\n\t"
    : "+r"(event_type_reg), "+r"(cur_sp_reg)
    );
    event_type = event_type_reg;
    switch( event_type ) {
        case UART1_OUT:
            *( (unsigned int*)(UART1_BASE + UART_CTLR_OFFSET)) |= TIEN_MASK | MSIEN_MASK;
            break;
        case UART1_IN:
            *( (unsigned int*)(UART1_BASE + UART_CTLR_OFFSET)) |= RIEN_MASK;
            break;
        case UART2_OUT:
            *( (unsigned int*)(UART2_BASE + UART_CTLR_OFFSET)) |= TIEN_MASK;
            break;
        case UART2_IN:
            *( (unsigned int*)(UART2_BASE + UART_CTLR_OFFSET)) |= RIEN_MASK | RTIEN_MASK;
            break;

        default:
            break;
    }
    if(DEBUG_SYSCALL)
        bwprintf(COM2,"SYSCALL: awaitEvent %d , event_type: %d \n\r", k_state->current_tid,event_type);
    k_state->event2td[event_type] = k_state->current_tid;
    //bwprintf(COM2,"SYSCALL: Value: %d \n\r", k_state->event2td[UART2_IN]);
    k_state->tasks_td[k_state->current_tid].task_state = BLOCKED;
}


void k_getIdle(KD *k_state){
    k_state->tasks_td[k_state->current_tid].task_return_value = ((k_state->idle_time * 100) / k_state->time);
    TD_unblock(k_state->tasks_td, k_state->current_tid, &(k_state->tasks_queue));
}

void k_getTrackID(KD *k_state){
    k_state->tasks_td[k_state->current_tid].task_return_value = k_state->track_id;
    TD_unblock(k_state->tasks_td, k_state->current_tid, &(k_state->tasks_queue));
}

