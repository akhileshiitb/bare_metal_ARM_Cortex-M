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
extern void _dsb(void);

void system_pendSV_call(uint32_t pendSV_number);

// global variables 
// Hold number of systick interrupts
uint64_t gTicks = 0; 

// Hard fault counter: hold number of hard faults generated
uint32_t gHardFault_counter = 0; 

// Usage fault counter: hold number of hard faults generated
uint32_t gUsageFault_counter = 0; 

// Usage counter for pendSV call 
uint32_t gPendSV_call_counter = 0; 

// Counter for NMI faults
uint32_t gNmi_counter = 0; 

// counter for external interrupt 0
uint32_t gInt0_counter = 0; 

// memManage fault counter 
uint32_t gMemFault_counter = 0; 

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
// interrupt control and state register
#define ICSR	*(uint32_t *)0xE000ED04
// NVIC registers 
#define NVIC_IPR0 *(uint32_t *)0xE000E400U // Interrupt priority
#define NVIC_ISER0	*(uint32_t *)0xE000E100U // Interrupt set enable 
#define NVIC_ISPR0	*(uint32_t *)0xE000E200U // Interrupt set prnding

// MPU PMSAV7 registers
typedef struct mpu_registers_t 
{
		volatile uint32_t MPU_TYPE; // type
		volatile uint32_t MPU_CTRL; // control 
		volatile uint32_t MPU_RNR; // Region number
		volatile uint32_t MPU_RBAR; // Region base address
		volatile uint32_t MPU_RASR; // Attribute and size
}mpu_regs; 

static mpu_regs* ptr_mpu_regs = (mpu_regs *) 0xE000ED90;


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

		// Call SVC . As we have defined SVC priority more than systick, systick should preempt. 
		// This checks out nested vectoring
		_system_svc_call(0x0);

		// call pendSV 
		system_pendSV_call(0x0U);
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

		// Configure external interrupt 0 via NVIC
		// Set priority and enable external interrupt 0
		NVIC_IPR0 |= (0x0AU << 0); // pririty 0xA

		// Enable  external interrupt 0
		NVIC_ISER0 |= (0x1U<<0); 

		return ret; 

}

/* *
 * Function to trigger pendSV exception
 * */
void system_pendSV_call(uint32_t pendSV_number)
{
		// set pending status of pendSV in ICSR register
		ICSR |= (1U<<28U);
}

/* *
 * PendSV handler: can be used to trigger context switch
 * */
void system_pendSV_handler(uint32_t pendSV_number)
{
		// increment pendSV counter 
		gPendSV_call_counter++; 
		
		// execute the required pendSV service
		switch(pendSV_number)	
		{
				case 0: //TODO 
						break; 
				case 1: //TODO
						break; 
				case 2: //TODO 
						break; 
				default:
						break; 
		}
}

/* *
 * Function which trigerres NMI exception
 * */
void system_trigger_nmi()
{
		ICSR |= (1U << 31U);
}

void system_nmi_handler()
{
		gNmi_counter++; 
}

/* *
 * Function to trigger external interrupt 0 
 * */
void system_trigger_interrupt(uint32_t int_num)
{
		if (int_num < 31)
		{
				NVIC_ISPR0 |= (0x1U << int_num);
		}
}

void system_ext_interrupt0_handler()
{
	// increament counter
	gInt0_counter++;	
}

static uint32_t mpu_get_num_regions()
{
		uint32_t num_regions = 0; 
		num_regions = ptr_mpu_regs->MPU_TYPE;  
		num_regions &= (0xFFU << 8U); 
		num_regions = num_regions >> 8U; 
		return num_regions; 
}

static uint32_t mpu_enable()
{
		ptr_mpu_regs->MPU_CTRL |= (1U << 0U); 
		_dsb(); // data sync barrier
		_isb(); // instuction sync barrier
		return 0xAAU; 
}

static uint32_t mpu_disable()
{
		ptr_mpu_regs->MPU_CTRL &= ~(1U << 0U); 
		_dsb(); // data sync barrier
		_isb(); // instuction sync barrier
		return 0xAAU; 
}

/* Function enabled defalut memory map as backgound region for Priv mode */
static uint32_t mpu_enable_priv_default()
{
		ptr_mpu_regs->MPU_CTRL |= (1U << 2U);
		return 0xAAU; 
}
/* Function disables default memory for priv access as backgorund region */
static uint32_t mpu_disable_priv_default()
{
		ptr_mpu_regs->MPU_CTRL &= ~(1U << 2U); 
		return 0xAAU; 
}

static void mpu_select_region(uint8_t region_num)
{
		ptr_mpu_regs->MPU_RNR = region_num; 
}

static void mpu_set_region_base_addr(uint32_t base_addr)
{
		ptr_mpu_regs->MPU_RBAR = (base_addr >> 5U) << 5U; // minimum alignment 
}

static void mpu_set_region_access_permission(uint8_t ap)
{
		ptr_mpu_regs->MPU_RASR &= ~(0b111 << 24U);
		ptr_mpu_regs->MPU_RASR |= ((ap & 0b111) << 24U);
}

static void mpu_set_region_execute_permission(uint8_t xn)
{
		ptr_mpu_regs->MPU_RASR &= ~(1<< 28U);
		ptr_mpu_regs->MPU_RASR |= ((xn & 0x1) << 28U);
}

static void mpu_set_region_size(uint8_t size)
{
		ptr_mpu_regs->MPU_RASR &= ~(0b11111 << 1U);
		ptr_mpu_regs->MPU_RASR |= ((size & 0b11111) << 1); 
}

static void mpu_region_enable()
{
		ptr_mpu_regs->MPU_RASR |= 0b1; 
		_dsb();
		_isb();
}

static void mpu_region_disable()
{
		ptr_mpu_regs->MPU_RASR &= ~(0b1);
}

static void mpu_set_attr_tex_s_c_b(uint8_t value)
{
		ptr_mpu_regs->MPU_RASR &= ~(0b111111 << 16U);
		ptr_mpu_regs->MPU_RASR |= (value << 16U);
}
/* *
 * Access permission: 0b110 priv/unpriv read only 
 * No writes allowed
 * */
static void mpu_set_ap_read_only()
{
		mpu_set_region_access_permission(0b110U);
}
/* *
 * Acecss permission: 0b001
 * Read/write only to priv
 * no access on unpriv
 * */

static void mpu_set_ap_priv_rw()
{
		mpu_set_region_access_permission(0b001U);
}
/* *
 * Acecss permission: 0b101
 * Read/write only to priv
 * no access on unpriv
 * */

static void mpu_set_ap_priv_r()
{
		mpu_set_region_access_permission(0b101U);
}
/* *
 * Access permission: r/w to both priv/unpriv mode.
 * */
static void mpu_set_ap_full()
{
		mpu_set_region_access_permission(0b011U);

}

/* Function to initilize static MPU
 * MPU configuration is as follows: 
 * Enables privildged backgournd region to access defalut memory map 
 *
 * region 1: 
 * region 2: 
 *
 * */
uint32_t system_mpu_init()
{
		uint32_t mpu_regions, ret; 
		mpu_regions = mpu_get_num_regions();
		if (mpu_regions == 0x0U)
		{
				// MPU is not supported, return error
				ret = 0xFF; 
				return ret; 
		}

		// MPU init
		mpu_enable_priv_default(); // enable priviledged background region: default system map

		// This should be the last statment
		mpu_enable();

}

void system_mem_manage_fault_handler()
{
		gMemFault_counter++; 
}

void system_mpu_tests()
{
		uint32_t temp_var; 

		// get some memory 0x20000000 + 32KB offset is safe to take
		void * mem_block_1_4KB = (void *)0x20008000U; // 4KB memory block
		void * mem_block_2_4KB = (void *)(0x20008000U + (1024*4)); // 4KB memory block

		// test 1: 
		// block 1 : read only block 2: full r/w
		// REGION 0
		system_enter_priv(); // MPU configs needs to be done from priv state
		// select MPU region 0
		mpu_select_region(0x0);
		// disable region before doing configs
		mpu_region_disable();
		// set region 0 base address
		mpu_set_region_base_addr((uint32_t)mem_block_1_4KB);
		// set region size
		mpu_set_region_size(11U); // 4KB size pow(2, 11+1)
		// set execute permission: no execute : XN bit
		mpu_set_region_execute_permission(0x1U);
		// set access permission
		mpu_set_ap_read_only();
		// set region attribute Normal outer/inner non chacable, sharable
		mpu_set_attr_tex_s_c_b(0b001100);
		// enable MPU region
		mpu_region_enable();
		
		// REGION 1
		// select MPU region 1
		mpu_select_region(0x1);
		// disable region before doing configs
		mpu_region_disable();
		// set region 1 base address
		mpu_set_region_base_addr((uint32_t)mem_block_2_4KB);
		// set region size
		mpu_set_region_size(11U); // 4KB size pow(2, 11+1)
		// set execute permission: no execute : XN bit
		mpu_set_region_execute_permission(0x1U);
		// set access permission
		mpu_set_ap_full();
		// set region attribute Normal outer/inner non chacable, sharable
		mpu_set_attr_tex_s_c_b(0b001100);
		// enable MPU region
		mpu_region_enable();

		// REGION 2: all code/text full enable
		// select MPU region 2
		mpu_select_region(0x2);
		// disable region before doing configs
		mpu_region_disable();
		// set region 1 base address
		mpu_set_region_base_addr((uint32_t)0x0); // .text section is 0x0
		// set region size
		mpu_set_region_size(13U); // 16KB size pow(2, 13+1)
		// set execute permission: execute : XN bit
		mpu_set_region_execute_permission(0x0U);
		// set access permission
		mpu_set_ap_full();
		// set region attribute Normal outer/inner non chacable, sharable
		mpu_set_attr_tex_s_c_b(0b001100);
		// enable MPU region
		mpu_region_enable();

		// REGION 3: Full stack PSP/MSP full access no exec
		// select MPU region 3
		mpu_select_region(0x3);
		// disable region before doing configs
		mpu_region_disable();
		// set region 1 base address
		mpu_set_region_base_addr((uint32_t)0x2000E000); // stack base
		// set region size
		mpu_set_region_size(12U); // 8KB size pow(2, 12+1)
		// set execute permission: execute : XN bit: stack no exec
		mpu_set_region_execute_permission(0x1U);
		// set access permission
		mpu_set_ap_full();
		// set region attribute Normal outer/inner non chacable, sharable
		mpu_set_attr_tex_s_c_b(0b001100);
		// enable MPU region
		mpu_region_enable();
		
		// REGION 4: all data full enable
		// select MPU region 4
		mpu_select_region(0x4);
		// disable region before doing configs
		mpu_region_disable();
		// set region 1 base address
		mpu_set_region_base_addr((uint32_t)0x20000000); // .data at 0x20000000
		// set region size
		mpu_set_region_size(12U); // 8KB size pow(2, 12+1)
		// set execute permission: execute : XN bit
		mpu_set_region_execute_permission(0x0U); // Data not executable
		// set access permission
		mpu_set_ap_full();
		// set region attribute Normal outer/inner non chacable, sharable
		mpu_set_attr_tex_s_c_b(0b001100);
		// enable MPU region
		mpu_region_enable();

		// TEST: try writing in block2  (block 2 is Read/Write)
		*(uint32_t *)mem_block_2_4KB = 0xFF; // should pass
		// TEST: try reading block 2 data 
		temp_var = *(uint32_t *)mem_block_2_4KB;  // should pass

		// TEST: try reading block 1 data  (block 1 is read only)
		temp_var = *(uint32_t *)mem_block_1_4KB;  // should pass
		// TEST: try writing in block1 
		*(uint32_t *)mem_block_1_4KB = 0xFF; // should fail: should generate memManage fault

		// Now change mem block 2 as Read only priviledged
		// REGION 1
		// select MPU region 1
		mpu_select_region(0x1);
		// disable region before doing configs
		mpu_region_disable();
		// set region 1 base address
		mpu_set_region_base_addr((uint32_t)mem_block_2_4KB);
		// set region size
		mpu_set_region_size(11U); // 4KB size pow(2, 11+1)
		// set execute permission: no execute : XN bit
		mpu_set_region_execute_permission(0x1U);
		// set access permission
		mpu_set_ap_priv_r();
		// set region attribute Normal outer/inner non chacable, sharable
		mpu_set_attr_tex_s_c_b(0b001100);
		// enable MPU region
		mpu_region_enable();

		// tests on modifier block 2: read only priviledged
		// Current mode: Priviledged
		temp_var = *(uint32_t *)mem_block_2_4KB; // should pass

		// Switch to unpriviledged state
		system_enter_unpriv();
		temp_var = *(uint32_t *)mem_block_2_4KB; // should Fail
		// Switch back to priviledged state
		system_enter_priv();
		temp_var = *(uint32_t *)mem_block_2_4KB; // should Pass
		*(uint32_t *)mem_block_2_4KB = 0xDD; // should Fail

		mpu_disable();
		
}


void system_print_serial(uint8_t * str)
{
		/* Prints string */
		while (*str != '\0')
		{
				/* 0x4000C000 is address of UART TXD of QEMU  machine model */
				*(uint32_t *)0x4000C000 = *str ; 
				str++; 
		}
}

