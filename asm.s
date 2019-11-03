.global _TimerIRQStackBase
.global asm_SetUpIRQStack
asm_SetUpIRQStack:
MRS r7, CPSR
AND r6, r6, #0
ORR r6, r6, #18
MSR CPSR, r6
LDR SP, [PC, #4]
MSR CPSR, r7
BX LR
_TimerIRQStackBase:
.4byte  0x01C00000

.align 2
.global kernel_exit
.type kernel_exit, %function
kernel_exit:
  mov ip, sp
  stmfd sp!, {r4-r12, r14}
  stmfd sp!, {r3}
  msr spsr, r2
  ldmfd r1!, {r14}
  msr cpsr_c, #223
  mov sp, r1
  ldmfd sp!, {r1-r12, r14}
  msr cpsr_c, #211
  movs pc, r14

.global asm_k_hwi
asm_k_hwi:
  msr cpsr_c, #211
  stmfd sp!, {r0-r2}
  mov r2, #0x64
  add r1, sp, #12
  ldmfd r1, {r0}
  str r2, [r0]
  msr cpsr_c, #146
  mov r0, lr
  mrs r1, spsr
  msr cpsr_c, #211
  sub lr, r0, #4
  msr spsr, r1
  ldmfd sp!, {r0-r2}

.global asm_k_swi
asm_k_swi:
  msr cpsr_c, #223
  stmfd sp!, {r1-r12, r14}
  mov r3, r0
  mov r1, sp
  msr cpsr_c, #211
  ldr r0, [lr,#-4]
  bic r0, r0,#0xFF000000
  stmfd r1!, {lr}
  mrs r2, spsr
  add sp, sp, #4
  ldmfd sp!, {r4-r12, r14}
  mov sp, ip
  mov pc, lr
