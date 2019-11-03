#include "kernel_hwi.h"
#include "kernel_swi.h"

void hwi_handle(KD *k_state) {
	if(*VIC1BaseAddress & (1 << 25)){
		uart2inHandler(k_state);
	}
	if(*VIC1BaseAddress & (1 << 26)){
		uart2outHandler(k_state);
	}
	if(*VIC2BaseAddress & (1 << (TC3OI - 32))){
		timerHandler(k_state);
	}	
	if(*VIC2BaseAddress & (1 << (INT_UART1 - 32))){
		uart1Handler(k_state);
	}
}

void hwi_initial(){
	*VIC1IntSelect = 0;
	*VIC1IntEnable |= 1 << UART2RXINTR2;
	*VIC1IntEnable |= 1 << UART2TXINTR2;
	*VIC2IntSelect = 0;
	*VIC2IntEnable |= 1 << (TC3OI - 32);
	*VIC2IntEnable |= 1 << (INT_UART1 - 32);
}

void timerHandler(KD *k_state) {
	*timer_clear = 1;
    k_state->time++;
    if(k_state->current_tid == 5){
        k_state->idle_time ++;
    }
	if(k_state->tasks_td[k_state->event2td[CLOCK]].task_state==BLOCKED) {
		TD_unblock(k_state->tasks_td, k_state->event2td[CLOCK], &(k_state->tasks_queue));
	}
	else{
		bwprintf(COM2, "Clock notifier is %d \n\rClock server condition is %d\n\r", k_state->tasks_td[6].task_state,k_state->tasks_td[2].task_state);
	}
}

void uart1Handler(KD *k_state) {
 	//bwprintf(COM2,"UART1  FIREDD!!!!!!\n\r");
	int *UART1INT = ((int*)(UART1_BASE + UART_INTR_OFFSET));
	int *UART1Ctrl = (int*) (UART1_BASE + UART_CTLR_OFFSET);
	int *flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
    int *status;
    //receive interrupt
	if (RIS_MASK & *UART1INT && *flags & RXFF_MASK) {
		//bwprintf(COM2,"UART1  RECEIVE FIREDD!!!!!!\n\r");
		status = (int *)( UART1_BASE + UART_RSR_OFFSET );
		if((*status & OE_MASK))
			bwprintf(COM2,"OE2 ERROR\n\r");
		*UART1Ctrl &= ~RIEN_MASK;
		*UART1INT &= ~RIS_MASK;
		TD_unblock(k_state->tasks_td, k_state->event2td[UART1_IN], &(k_state->tasks_queue));
	}
	//transmit interrupt
	if ((* UART1INT & TIS_MASK)&& *flags & TXFE_MASK) { // UART_RTIS_CHECKER
		*UART1Ctrl &= ~TIEN_MASK;
		k_state->UART1_TBE = 1;
	}
	//modem interrupt
	if ((*UART1INT & MIS_MASK) && *flags & CTS_MASK && k_state->UART1_TBE) {
		*UART1Ctrl &= ~MSIEN_MASK;
		k_state->UART1_C2S = 1;
	}
	if (k_state->UART1_TBE == 1 && k_state->UART1_C2S == 1) { // UART_RTIS_CHECKER
		k_state->UART1_TBE = 0;
		k_state->UART1_C2S = 0;
		*UART1Ctrl &= ~TIEN_MASK;
		*UART1Ctrl &= ~MSIEN_MASK;
		TD_unblock(k_state->tasks_td, k_state->event2td[UART1_OUT], &(k_state->tasks_queue));
	}
}

void uart2inHandler(KD *k_state) {
	//bwprintf(COM2,"UART2IN  FIREDD!!!!!!\n\r");
	int * flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
	if(	*flags & RXFF_MASK){
		int * UART2Ctrl = (int*) (UART2_BASE | UART_CTLR_OFFSET);
		*UART2Ctrl &= ~RIEN_MASK;
		//bwprintf(COM2, "UART2 Interrupt Fired...Unblocking %d..\n\r",k_state->event2td[4]);
		TD_unblock(k_state->tasks_td, k_state->event2td[UART2_IN], &(k_state->tasks_queue));
	}
	return;
}

void uart2outHandler(KD *k_state) {
	//bwprintf(COM2,"UART2OUT  FIREDD!!!!!!\n\r");
	int *flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
	if(*flags & TXFE_MASK){
		int * UART2Ctrl = (int*) (UART2_BASE | UART_CTLR_OFFSET);
		*UART2Ctrl &= ~TIEN_MASK;
		TD_unblock(k_state->tasks_td, k_state->event2td[UART2_OUT], &(k_state->tasks_queue));
	}
	return;
}
