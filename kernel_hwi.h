#ifndef HWI_H_
#define HWI_H_
#include "bwio.h"
#include "print.h"
#include "kernel_swi.h"
#include "ts7200.h"

volatile static int * const timer_ldr = (int*)(TIMER3_BASE + LDR_OFFSET);
volatile static int * const timer_val = (int*)(TIMER3_BASE + VAL_OFFSET);
volatile static int * const timer_ctrl = (int*)(TIMER3_BASE + CRTL_OFFSET);
volatile static int * const timer_clear = (int*)(TIMER3_BASE + CLR_OFFSET);
volatile static const int TC3OI = 51;
volatile static const int INT_UART1 = 52;
volatile static const int INT_UART2 = 54;
volatile static const int UART1RXINTR1 = 23;
volatile static const int UART1TXINTR1 = 24;
volatile static const int UART2RXINTR2 = 25;
volatile static const int UART2TXINTR2 = 26;
volatile static int * const VIC1BaseAddress = (int *) 0x800B0000;
volatile static int * const VIC1IntSelect = (int *)0x800B000C;
volatile static int * const VIC1IntEnable = (int *)0x800B0010;
volatile static int * const VIC1VectAddr = (int *)0x800B0030;
volatile static int * const VIC1IntEnClear = (int *)0x800B0014;
volatile static int * const VIC1SoftInt = (int*) 0x800B0018;
volatile static int * const VIC1SoftIntClear = (int*) 0x800B001C;
volatile static int * const VIC2BaseAddress = (int *) 0x800C0000;
volatile static int * const VIC2IntSelect = (int *)0x800C000C;
volatile static int * const VIC2IntEnable = (int *)0x800C0010;
volatile static int * const VIC2VectAddr = (int *)0x800C0030;
volatile static int * const VIC2IntEnClear = (int *)0x800C0014;
volatile static int * const VIC2SoftInt = (int*) 0x800C0018;
volatile static int * const VIC2SoftIntClear = (int*) 0x800C001C;

void hwi_initial();
void hwi_handle(KD *k_state);
EVENT hwi_event();
void hwi_activate(EVENT event);
void timerHandler(KD *k_state);
void uart1Handler(KD *k_state);
//void uart2Handler();
void uart2inHandler(KD *k_state);
void uart2outHandler(KD *k_state);
#endif
