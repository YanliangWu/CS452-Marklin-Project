#ifndef MAIN_H_
#define MAIN_H_
void asm_SetUpIRQStack();
extern int _TimerIRQStackBase;
void asm_k_swi();
void asm_k_hwi();
void kernel_exit(int retval, unsigned int *sp, unsigned int spsr, int *hwi_flag);

#endif
