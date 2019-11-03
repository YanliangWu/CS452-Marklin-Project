// print.c is the printing module use for checking register values
// print module is generally for testing only.

#include <ts7200.h>
#include "print.h"
#include "bwio.h"

//---------------------debug print-----------------------
/*
void print_r8(void){
	register int r8 asm("r8");
	bwprintf(COM2,"\n\rRegister R8:0x%x\n\r",r8);
}
void print_r9(void){
	register int r9 asm("r9");
	bwprintf(COM2,"\n\rRegister R9:0x%x\n\r",r9);
}
void print_r6(void){
	register int r6 asm("r6");
	bwprintf(COM2,"\n\rRegister R6:0x%x\n\r",r6);
}
*/



//---------------------kernel print----------------------
void print_intial(){
	bwprintf(COM2, "\x1B""[2J");
	bwprintf(COM2, "\033[?25l");
	bwprintf(COM2, "\33[1;1f");
	bwprintf(COM2, "============================================================================================================\n\r");
	bwprintf(COM2, "                                   CS 452 Control Interface (Reborn)                                        \n\r");
	bwprintf(COM2, "============================================================================================================\n\r");
	bwprintf(COM2, "  Current time               Idle Rate                   Track                     Command                  \n\r");
	bwprintf(COM2, "=============================================\n\r");
	bwprintf(COM2, "                    Switch                   \n\r");
	bwprintf(COM2, "  1   2   3   4   5   6   7   8   9   10  11 \n\r");
	bwprintf(COM2, "  C   C   C   C   C   C   C   C   C   S   C  \n\r"); // 8
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "  12  13  14  15  16  17  18 153 154 155 156 \n\r");
	bwprintf(COM2, "  C   S   C   C   S   S   C   C   C   C   C  \n\r"); // 11
	bwprintf(COM2, "=============================================\n\r");
	bwprintf(COM2, "                    Sensor                   \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "=============================================\n\r");
	bwprintf(COM2, "  >                                          \n\r");
	bwprintf(COM2, "=============================================\n\r");
	bwprintf(COM2, "                   Schedule                  \n\r");
	bwprintf(COM2, " NUM FET SPE DES POS DIST VEL                \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "                                             \n\r");
	bwprintf(COM2, "============================================================================================================\n\r");
	bwprintf(COM2, "\33[23;6f");
}

void print_load(){
	bwprintf(COM2, "\x1B""[2J");
	bwprintf(COM2, "\033[?25l");
	bwprintf(COM2, "\33[1;1f");
	bwprintf(COM2, "=============================================================================================================\n\r");
	bwprintf(COM2, "                                            Loading......                                           \n\r");
	bwprintf(COM2, "=============================================================================================================\n\r");
}
