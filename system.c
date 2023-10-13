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
extern uint32_t _set_faultmask(uint32_t faultmask);
extern uint32_t _set_basepri(uint32_t basepri);
extern uint32_t _set_control(uint32_t set_control);
extern uint32_t _set_control_handler(uint32_t set_control);

extern void _isb(void);

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


