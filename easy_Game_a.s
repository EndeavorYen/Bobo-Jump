	AREA easy_Game_a, CODE, READONLY
	
	IMPORT game_main
	IMPORT timer_irq
	ENTRY

Vector_init_Block
	LDR PC, Reset_Addr
	LDR PC, Undefined_Addr
	LDR PC, SWI_Addr
	LDR PC, Prefetch_Addr
	LDR PC, Abort_Addr
	NOP
	LDR PC, IRQ_Addr
	LDR PC, FIQ_Addr
  
Reset_Addr		DCD Reset_Handler
Undefined_Addr 	DCD Undefined_Handler
SWI_Addr		DCD SWI_Handler
Prefetch_Addr	DCD Prefetch_Handler
Abort_Addr		DCD Abort_Handler
				DCD 0
IRQ_Addr		DCD IRQ_Handler
FIQ_Addr		DCD FIQ_Handler

Reset_Handler
; Enter IRQ mode and set up the IRQ stack pointer
	MOV r0, #(0x12 | 0x80 | 0x40)         ; 0x12 = IRQ Mode, 0x80 = IRQ disable,  0x40 = disable FIQ
	MSR cpsr_c, r0                        ; Enter IRQ Mode
	LDR r1, =0x00074000                   ; Set IRQ Stack
	MOV sp, r1
	SUB r1, r1, #0x5000
; Enter SVC mode and set up the SVC stack pointer
	MOV r0, #(0x13 | 0x80 | 0x40)         ; 0x13 = SVC Mode
	MSR cpsr_c, r0                        ; Enter SVC Mode
	MOV sp, r1                            ; Set SVC Stack
	SUB r1, r1, #0x5000
; Enter User Mode
	MOV r0, #(0x10 | 0x40)                ; 0x10 = User Mode
	MSR cpsr_c, r0                        ; Enter User Mode
	B game_main                              ; Branch to C function

Undefined_Handler

SWI_Handler

Prefetch_Handler

Abort_Handler

IRQ_Handler
	STMFD sp!, {r0-r12, lr}               ; Storage registers
	BL timer_irq                          ; Branch to ISR
	LDMFD sp!, {r0-r12, lr}               ; Restore registers
	SUBS pc, lr, #4                        ; Return to Function
	
FIQ_Handler 
	
	END