#include "initialization.h"
#include "main.h"
#include "kernel_hwi.h"
#include "global.h"
#include "nameserver.h"
#include "clockserver.h"
#include "trainserver.h"



void cache_init(){
    asm volatile(
    "MRC p15, 0, a1, c1, c0, 0\n\t"
            "ORR a1, a1, #0x5\n\t"
            "ORR a1, a1, #0x1000\n\t"
            "MCR p15, 0, a1, c1, c0, 0\n\t"
    );
}

void kernel_Boot(KD *k_state){
    k_state->redboot_sp = k_state->user_sp;
    k_state->redboot_pc = k_state->user_pc;
    k_state->redboot_cpsr = k_state->user_cpsr;
    k_state->tasks_count = 0;
    k_state->init_flag = 0;
    k_state->time = 0;
    k_state->idle_time = 0;
    k_state->UART1_C2S = 0;
    k_state->UART1_TBE = 0;
    priority_queue_initial(&(k_state->tasks_queue));
    TD_initial(k_state->tasks_td,MAX_TASK_NUM);

    TD_create(k_state->tasks_td,&ns_server_boot,0,1,&(k_state->tasks_queue));       //1
    TD_create(k_state->tasks_td,&clock_server_boot,0,3,&(k_state->tasks_queue));    //2
    TD_create(k_state->tasks_td,&uart_server_boot,0,6,&(k_state->tasks_queue));     //3
    TD_create(k_state->tasks_td,&train_server_boot,0,8,&(k_state->tasks_queue));    //4
    TD_create(k_state->tasks_td,&clock_idle_boot,0,10,&(k_state->tasks_queue));     //5

    k_state->tasks_count = 5;
}

void syscall_handler(KD *k_state, int syscall_type) {
        switch( syscall_type ) {
        case HWI_INTERRUPT:
            hwi_handle(k_state);
            TD_unblock(k_state->tasks_td, k_state->current_tid, &(k_state->tasks_queue));
            break;
        case SYSCALL_CREATE:
            k_Create(k_state);
            break;
        case SYSCALL_MYTID:
            k_MyTid(k_state);
            break;
        case SYSCALL_MYPARENTTID:
            k_MyParentTid(k_state);
            break;
        case SYSCALL_SEND:
            k_Send(k_state);
            break;
        case SYSCALL_RECEIVE:
            k_Receive(k_state);
            break;
        case SYSCALL_REPLY:
            k_Reply(k_state);
            break;
        case SYSCALL_PASS:
            k_Pass(k_state);
            break;
        case SYSCALL_AWAITEVENT:
            k_awaitEvent(k_state);
            break;
        case SYSCALL_EXIT:
            k_Exit(k_state);
            break;
        case SYSCALL_GETIDLE:
            k_getIdle(k_state);
            break;
        case SYSCALL_GETTRACKID:
            k_getTrackID(k_state);
            break;
        default:
            break;
    }
}

int task_enter(KD *k_state, TD *td) {

    // save scratch regtisters....
    register int syscall_type_reg  asm("r0");
    register unsigned int *sp_reg asm("r1");
    register unsigned int new_spsr_reg asm("r2");
    register int user_r0_reg  asm("r3");
    int request_type;
    int hwi_request_flag = -1;

    k_state->current_tid = td->task_tid;
    kernel_exit(td->task_return_value, td->task_sp, td->task_cpsr, &hwi_request_flag);

    // Welcome Back..
    asm volatile(
    "mov %0, r0\n\t"
    "mov %1, r1\n\t"
    "mov %2, r2\n\t"
    "mov %3, r3\n\t"
    : "+r"(syscall_type_reg), "+r"(sp_reg),
    "+r"(new_spsr_reg), "+r"(user_r0_reg)
    );

    // save to TD..
    request_type = syscall_type_reg;
    td->task_sp = sp_reg;
    td->task_cpsr = new_spsr_reg;
    td->task_return_value = user_r0_reg;

    // 0x64 is what I hardcoded for HWI.
    if( hwi_request_flag == 0x64) {
        request_type = HWI_INTERRUPT;
    }
    hwi_request_flag = -1;
    return request_type;
}


void switch_init(){
    int j;
    for (j = 1; j < 18; j++) {
        if(j!=10&&j!=13&&j!=16&&j!=17) {
            bwputc(COM1, 34);
            bwputc(COM1, j);
            bwputc(COM1, 32);
        }
    }
    for (j = 153; j < 157; j++) {
        bwputc(COM1, 34);
        bwputc(COM1, j);
        bwputc(COM1, 32);

    }
    bwputc(COM1, 33);
    bwputc(COM1, 10);
    bwputc(COM1, 32);
    bwputc(COM1, 33);
    bwputc(COM1, 13);
    bwputc(COM1, 32);
    bwputc(COM1, 33);
    bwputc(COM1, 16);
    bwputc(COM1, 32);
    bwputc(COM1, 33);
    bwputc(COM1, 17);
    bwputc(COM1, 32);
    return;
}

int main( int argc, char* argv[] ) {

    channel com1;
    channel com2;
    set_channel(&com1,&com2);

    //set the interrupt
    int *first_swi_instruction_address = (int *)(0x28);
    *first_swi_instruction_address = (int)&asm_k_swi;
    int *first_hwi_instruction_address = (int *)(0x38);
    *first_hwi_instruction_address = (int)&asm_k_hwi;

    int * timerirq_stack_base = &_TimerIRQStackBase;
    *timerirq_stack_base = TIMER_IRQ_STACK_START;
    asm_SetUpIRQStack();


    int request_type;
    KD k_state;
    kernel_Boot(&k_state);
    cache_init();

    bwprintf(COM2, "\x1B""[2J");
    bwprintf(COM2, "\33[1;1f");
    bwprintf(COM2, "Select Track ID: (a/b)...........\n\r");
    int track_id = bwgetc(COM2);
    bwprintf(COM2,"Initializing Track %c ...... \n\r",track_id);
    k_state.track_id = track_id;
    switch_init();


    print_intial(); // Loading Finished!!
    while (1) {
        int next_tid = TD_next( &(k_state.tasks_queue));
        if ( next_tid == 0 ) {
            break;
        }
        TD * next_task = &(k_state.tasks_td[next_tid]);
        //bwprintf(COM2, "Scheduler scheduled next task: %d, activating...\n\r", next_tid);
        request_type = task_enter(&k_state, next_task);
        //bwprintf(COM2, "Getting into the syscall handler.....\n\r");
        if(request_type == SYSCALL_TERMINATE){
            break;
        }
        syscall_handler(&k_state, request_type);
    }
    bwprintf(COM2, "\x1B""[2J");
    bwprintf(COM2, "\33[1;1f");
    bwprintf(COM2, "Exiting....Bye\n\r");
    return 0;
}
