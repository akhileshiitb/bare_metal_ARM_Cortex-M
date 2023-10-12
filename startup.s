.extern main 
.syntax unified 

.text
.align 2
.global _vector_table
_vector_table:
		.word 0x20001000 // inital value of stack pointer
		.word _start
		.word _nmi_handler
		.word _hard_fault

.text
.align 2
.global _start 
.thumb_func
_start:
		mov r0, #0
		mov r1, #0

		// set up the main stack pointer
		ldr r0, _main_stack_pointer_init
		mov sp, r0
		
		/* set up process stack pointer*/
		mrs r0, control
		// select process stack poiner
		orr r0, r0, #(1<<1)
		msr control, r0
		// init PSP
		ldr r0, _ps_stack_pointer_init
		mov sp, r0

		// switch back to use main stack pointer
		mrs r0, control
		and r0, r0, #~(1<<1)
		msr control, r0

		bl main
loop:
		bl loop

.align 2 
.global _hard_fault
.thumb_func
_hard_fault:
		b _hard_fault

.align 2 
.global _nmi_handler
.thumb_func
_nmi_handler:
		b _nmi_handler

.align 2 
.thumb 
.global _main_stack_pointer_init
_main_stack_pointer_init:
		.word 0x2000FFFF  // MSP at to of the RAM

.align 2 
.thumb 
.global _ps_stack_pointer_init
_ps_stack_pointer_init:
		.word 0x2000F000  // PSP at MSP - 4KB

.align 2
.thumb
.global _enter_unpriv
.thumb_func
.type _enter_unpriv, %function 
_enter_unpriv:
		mrs r0, control 
		orr r0, r0, #(1<<0)
		msr control, r0
		bx lr

.end

/*
Notes:
Exception vector of Cortex M4 should have righmost bit set for all vectors. This 
is to tell processor that this is thumb instructions. 
Hence when we define symbol add .thumb_func for all assembly functions. This will automatically
set lsbit to 1 whenever symbol is referred. see. _start and _hard_fault definition above. 

.syntax unified : asks GNU assembler to use unified syntax. 

jumping between priv-unpriv thread modes: 
you can jump from priv thread mode to unpriv thread by writin to control.bit0. But you can not jump 
back to priv from unpriv mode (its priv break). Hence you need to generate exception (eg. SVC) to 
jump in handler mode and come back in priv thread mode. 

*/
