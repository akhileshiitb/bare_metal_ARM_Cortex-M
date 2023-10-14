/* Assembly functions for various use cases*/
.syntax unified 


/*
Function to check if processor is in priviledged or unpriv mode: 

uint32_t _is_priv(void);

return:
1: priv mode
0: unpriv mode 
0xFF: Error

*/

.text 
.align 2 
.thumb 
.thumb_func
.global _is_priv
.type _is_priv, %function 
_is_priv:
		mrs r0, control 
		and r0, r0, #(1<<0)
		cmp r0, #1
		bne L1
		mov r1, #0
		b L2
L1:
		mov r1, #1
L2:
		mov r0, r1
		bx lr

/* 
function to get primask special CPU register
uint32_t _get_primask(void);
*/
.text 
.align 2 
.thumb 
.thumb_func
.global _get_primask
.type _get_primask, %function 
_get_primask:
		mrs r0, primask
		bx lr

/* 
function to get faultmask special CPU register
uint32_t _get_faultmask(void);
*/
.text 
.align 2 
.thumb 
.thumb_func
.global _get_faultmask
.type _get_faultmask, %function 
_get_faultmask:
		mrs r0, faultmask
		bx lr

/* 
function to get basepri special CPU register
uint32_t _get_basepri(void);
*/
.text 
.align 2 
.thumb 
.thumb_func
.global _get_basepri
.type _get_basepri, %function 
_get_basepri:
		mrs r0, basepri
		bx lr

/* 
function to get control CPU register
uint32_t _get_control(void);
*/
.text 
.align 2 
.thumb 
.thumb_func
.global _get_control
.type _get_control, %function 
_get_control:
		mrs r0, control 
		bx lr


/* 
function to set primask special CPU register
uint32_t _set_primask(uint32_t set_primask);
return: 
0xAA: success
0xFF: failure (when called from unpriviledged state)
*/
.text 
.align 2 
.thumb 
.thumb_func
.global _set_primask
.type _set_primask, %function 
_set_primask:
		// check if we are executing in priv state
		push {lr}
		push {r0}
		bl _is_priv
		cmp r0, #1
		bne L3
		pop {r0}
		msr primask, r0
		mov r0, #0xAA 
		b L4

L3:
		pop {r0}
		mov r0, #0xFF

L4:
		pop {lr}
		bx lr

/* 
function to set primask special CPU register
uint32_t _set_primask_raw(uint32_t set_primask);
This version of function does not check for execution mode
it just simply goes and writes. Always return success
return: 
0xAA: success
0xFF: failure (when called from unpriviledged state)
*/
.text 
.align 2 
.thumb 
.thumb_func
.global _set_primask_raw
.type _set_primask_raw, %function 
_set_primask_raw:
		// check if we are executing in priv state
		msr primask, r0
		mov r0,#0xAA 
		bx lr

/* 
function to set faultmask special CPU register
uint32_t _set_faultmask(uint32_t set_faultmask);
return: 
0xAA: success
0xFF: failure (when called from unpriviledged state)
*/
.text 
.align 2 
.thumb 
.thumb_func
.global _set_faultmask
.type _set_faultmask, %function 
_set_faultmask:
		// check if we are executing in priv state
		push {lr}
		push {r0}
		bl _is_priv
		cmp r0, #1
		bne L3
		pop {r0}
		msr faultmask, r0
		mov r0, #0xAA 
		b L4

/* 
function to set basepri special CPU register
uint32_t _set_basepri(uint32_t set_basepri);
return: 
0xAA: success
0xFF: failure (when called from unpriviledged state)
*/
.text 
.align 2 
.thumb 
.thumb_func
.global _set_basepri
.type _set_basepri, %function 
_set_basepri:
		// check if we are executing in priv state
		push {lr}
		push {r0}
		bl _is_priv
		cmp r0, #1
		bne L3
		pop {r0}
		msr basepri, r0
		mov r0, #0xAA 
		b L4

/* 
function to set control register
uint32_t _set_control(uint32_t set_control);
return: 
0xAA: success
0xFF: failure (when called from unpriviledged state)
*/
.text 
.align 2 
.thumb 
.thumb_func
.global _set_control
.type _set_control, %function 
_set_control:
		// check if we are executing in priv state
		push {lr}
		push {r0}
		bl _is_priv
		cmp r0, #1
		bne L3
		pop {r0}
		msr control, r0
		mov r0, #0xAA 
		b L4
/* 
function to set control register when executing in handler mode
handler mode is always proviledged, hence no need to call _is_priv() 
uint32_t _set_control_handler(uint32_t set_control_handler);
return: 
0xAA: success (This function always returns success)
0xFF: failure (when called from unpriviledged state)
*/
.text 
.align 2 
.thumb 
.thumb_func
.global _set_control_handler
.type _set_control_handler, %function 
_set_control_handler:
		msr control, r0
		mov r0, #0xAA 
		bx lr

.text
.align 2 
.thumb
.thumb_func
.global _isb
.type _isb, %function 
_isb:
		isb
		bx lr

.text 
.align 2 
.thumb 
.thumb_func
.global _system_select_msp
.type _system_select_msp, %function 
_system_select_msp:
		// make sure we do not use any stack here 
		mrs r0, control 
		and r0, r0, #~(1<<1)
		msr control, r0
		isb
		bx lr

.text 
.align 2 
.thumb 
.thumb_func
.global _system_select_psp
.type _system_select_psp, %function 
_system_select_psp:
		// make sure we do not use any stack here 
		mrs r0, control 
		orr r0, r0, #(1<<1)
		msr control, r0
		isb
		bx lr

.text 
.align 2 
.thumb
.thumb_func
.global _system_svc_call
.type _system_svc_call, %function
_system_svc_call:
		//svc #0x0
		.word 0b1101111100000000 // encoded format of above SVC instruction
		// Above encoding for SVC is taken from armv7-m architecture reference manual
	    bx lr	

.text
.align 2 
.thumb
.thumb_func
.global _trigger_usage_fault
.type _trigger_usage_fault, %function 
_trigger_usage_fault:
		.word 0xFFFFFFFF // undefined intruction
		bx lr
		
.end



