.global asm_k_swi
.global asm_k_hwi
.global asm_k_exit
.global asm_k_boot
.global asm_k_start
.global asm_k_terminate
.global asm_boot
.global asm_create
.global asm_mytid
.global asm_myparenttid
.global asm_pass
.global asm_exit
.global asm_receive
.global asm_send
.global asm_reply
.global asm_awaitevent
.global asm_activeevent
.global asm_getIdle
.global _TimerIRQStackBase
.global asm_SetUpIRQStack

asm_SetUpIRQStack:
MRS r7, CPSR  /* Save current mode */
AND r6, r6, #0 /*  clear the register */
ORR r6, r6, #18 /*  Set mode bit to 18 for irq mode */
MSR CPSR, r6
LDR SP, [PC, #4] /* Load the value that we want for the IRQ stack */
MSR CPSR, r7 /* Restore current mode */
BX LR
_TimerIRQStackBase:
.4byte  0x00000000  /*  In kern.c we will set this to the value it needs to be */


asm_k_swi:
    MSR cpsr, #159
    MOV ip, sp
    stmfd sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
    
    MOV r8, #0x01500000
    LDR r8, [r8, #0]
	STR sp, [r8, #0]
    
    MSR cpsr, #147
    STR lr, [r8, #4]
    MRS r9, spsr
    STR r9, [r8, #8]
    STR r0, [r8, #12]

    LDR r8, [lr, #-4]
    MOV sp, #0x00005000
    B swi_handle



asm_k_hwi:
    MSR cpsr, #159
    MOV ip, sp
    stmfd sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
    MOV r5, sp

    MSR cpsr, #146
    SUB lr, lr, #4
    MOV r6, lr
    MRS r7, spsr

    MSR cpsr, #147
    MOV r8, #0x01500000
    LDR r8, [r8, #0]
    
    STR r5, [r8, #0]
    STR r6, [r8, #4]
    STR r7, [r8, #8]
    STR r0, [r8, #12]

    MOV sp, #0x00005000
    B hwi_handle



asm_k_exit:
    MSR cpsr, #147
    MOV r8, #0x01500000
    LDR r8, [r8, #0]

    LDR r0, [r8, #12]

    LDR r9, [r8, #8]
    MSR spsr, r9
	LDR lr, [r8, #4]

    MSR cpsr, #159
    LDR sp, [r8, #0]
    ldmfd sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
    MSR cpsr, #147

    MOVS pc, lr



asm_k_boot:
    MOV r8, #0x01500000
    LDR r8, [r8, #0]
	
    STR sp, [r8, #0]
    STR lr, [r8, #4]
    MRS r9, spsr
    STR r9, [r8, #8]
    STR r0, [r8, #12]

    LDR r8, [lr, #-4]
   
    B swi_handle



asm_k_start:
    MSR cpsr, #211
    MOV r8, #0x01500000
    LDR r8, [r8, #0]

    LDR r0, [r8, #12]
    LDR r9, [r8, #8]
    MSR spsr, r9
	LDR lr, [r8, #4]

    MSR cpsr, #159
    LDR sp, [r8, #0]
    MSR cpsr, #211

    MOVS pc, lr

asm_k_terminate:
    MOV r8, #0x01500000
    LDR r8, [r8, #0]

    LDR r0, [r8, #12]
    
    LDR r9, [r8, #8]
    MSR spsr, r9
	LDR lr, [r8, #4]

    LDR sp, [r8, #0]

    MOV pc, lr


asm_boot:
	MOV ip, sp
	stmfd   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	swi 0
	ldmfd   sp, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}
asm_create:
	swi 1
    MOV pc, lr
asm_mytid:
	swi 2
    MOV pc, lr
asm_myparenttid:
	swi 3
    MOV pc, lr
asm_pass:
	swi 4
    MOV pc, lr
asm_exit:
	swi 5
    MOV pc, lr
asm_send:
	swi 6
    MOV pc, lr
asm_receive:
    swi 7
    MOV pc, lr
asm_reply:
    swi 8
    MOV pc, lr
asm_awaitevent:
    swi 9
    MOV pc, lr
asm_activeevent:
    swi 10
    MOV pc, lr
asm_getIdle:
    swi 11
    MOV pc, lr
