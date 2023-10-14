#include <stdint.h>

/* system.c
 * File implements low level system functions in C 
 * system.s gives access to special function registers. This file uses this to 
 * do various configurations for the system. 
 * */

/* External interfaces from system.s */
extern void _enter_unpriv();

extern uint32_t _get_primask();
extern uint32_t _get_faultmask();
extern uint32_t _get_basepri();
extern uint32_t _get_control();

extern uint32_t _set_primask(uint32_t primask);
extern uint32_t _set_primask_raw(uint32_t primask);
extern uint32_t _set_faultmask(uint32_t faultmask);
extern uint32_t _set_basepri(uint32_t basepri);
extern uint32_t _set_control(uint32_t set_control);
extern uint32_t _set_control_handler(uint32_t set_control);

extern void _system_svc_call(uint32_t svc_number);

extern void _isb(void);

// global variables 
// Hold number of systick interrupts
uint64_t gTicks = 0; 

// Hard fault counter: hold number of hard faults generated
uint32_t gHardFault_counter = 0; 

// Usage fault counter: hold number of hard faults generated
uint32_t gUsageFault_counter = 0; 

/* Systic register layout */
typedef struct systick_reg_t {
		volatile uint32_t SYSTICK_CSR; 
		volatile uint32_t SYSTICK_RVR;
		volatile uint32_t SYSTICK_CVR; 
		volatile uint32_t SYSTICK_CALIB;
}systick_reg; 

static systick_reg* ptr_systick = (systick_reg*) 0xE000E010; // pointer to systic MMR space

// SBC MMRs for exception handeling
#define SHCSR	*(uint32_t *)0xE000ED24
#define SHPR1	*(uint32_t *)0xE000ED18 
#define SHPR2	*(uint32_t *)0xE000ED1C 
#define SHPR3	*(uint32_t *)0xE000ED20

void system_svc_handler(uint32_t svc_num){
		switch (svc_num){
				case 0: 
						// TODO
						break; 

				case 1: 
						// TODO
						break; 
						
				case 2: 
						// SVC #2 allows shift to priviledged thread mode
						{
								uint32_t control_reg; 
								control_reg = _get_control(); 
								control_reg &= ~(1<<0); // set priv thread mode
								_set_control_handler(control_reg);
						}
						break; 

				defalut:
						break;
		}
}

/* Function to enter into Priviledged thread mode */
void system_enter_priv()
{
		// Generate SVC call #2 
		// This will take us in handler mode, which is
		// priviledged and alloes to modify control register
		_system_svc_call(2U);


}
/* Function to enter into unpriviledged thread mode */
void system_enter_unpriv()
{
		_enter_unpriv();
}

/* *
 * system function to configure systick and enable interrupt from it. 
 * */
void system_systick_config()
{
		// Read the TENMS field  of CALIB
		uint32_t tenms_count; 

		tenms_count = ptr_systick->SYSTICK_CALIB; 
		tenms_count &= 0xFFFFFF; // mask 24 lsb bits 

		// set Reload value register RVR to TENMS
		ptr_systick->SYSTICK_RVR = tenms_count; 

		// Select the clock source for systick: CPU clock
		ptr_systick->SYSTICK_CSR |= (1U << 2U);

		// enable the interrupt
		ptr_systick->SYSTICK_CSR |= (1U << 1U);

		// enable the systick counter
		ptr_systick->SYSTICK_CSR |= (1U << 0U);

}

void system_systick_handler()
{
		// ACK systick interrupt write to CVR and read CSR 
		volatile uint32_t csr_val; 
		csr_val = ptr_systick->SYSTICK_CSR; 

		// dummy write to CVR
		ptr_systick->SYSTICK_CVR = 0x0; 

		// increament systick counter
		gTicks++; 

		// Do systick handler tasks
}
/* Function enables system wide exceptions by clearing primask special function
 * register
 * This function can be called from handler or priviledged thread mode
 * */
uint32_t system_enable_exceptions()
{
		return _set_primask_raw(0x0U);
}

/* *
 * Function disbles system wide exceptions by setting primask special function 
 * register
 * This function can be called from handler or priviledged thread mode
 * */
uint32_t system_disable_exceptions()
{
		return _set_primask_raw(0x1U);
}

/* 
 * This function eanbles hard fault by wirting to FAULTMASK special function register
 * */
uint32_t system_enable_hardfault()
{
		return _set_faultmask(0x0U);
}
/* *
 * This function disables hardfault by wirintg to FAULT mask special function register
 * */
uint32_t system_disable_hardfault()
{
		return _set_faultmask(0x1U);
}

/* *
 * This function sets BASEPRI sepcial funtion register
 * sets the minumum priority level for processor to take exception. 
 * when 0x0 , does not have effect
 * when non-zero: masks out all expcetions with priority lower then value in basepri
 * This function should be called from priviledged thread mode only 
 * */
uint32_t system_set_basepri(uint32_t basepri)
{
		return _set_basepri(basepri);
}

uint32_t system_exceptions_init()
{
		uint32_t ret = 0; 
		// clear primask to enable all exceptions
		ret = system_enable_exceptions();
		if (ret == 0xAA)
		{
				// Enable Hard fault
				ret = system_enable_hardfault();
		}

		if (ret == 0xAA)
		{
				// set base priority to lowest level 
				ret = system_set_basepri(0xFF);
		}

		// Set priorities of system exceptions 
		// FAULTs -> SVC (High) -> SYSTICK -> PENSV
		
		// all faults : memMange, Bus faults, usage fault = 0x2U
		// Refer: ARMv7-m technical reference manual for register fields
		SHPR1 = (0x02U << 0U) | (0x02U << 8U) | (0x02U << 16U); 

		// SVC : 0x4U
		SHPR2 = (0x04U << 24U);

		// SYSTICK: 0x6U 
		SHPR3 |= (0x06U << 24U); 

		// PENDSV : 0x08U
		SHPR3 |= (0x08 << 16U);
																

		// Enable system fault exceptions
		// enable usage fault, Bus fault, memMange fault (bit 16,17,18 set)
		SHCSR |= (0b111 << 16U);

		return ret; 

}

