#include<stdio.h>
#include<stdint.h>

extern void _enter_unpriv();
extern uint32_t _get_primask();
extern uint32_t _get_faultmask();
extern uint32_t _get_basepri();
extern uint32_t _get_control();

extern uint32_t _set_primask(uint32_t primask);
extern uint32_t _set_faultmask(uint32_t faultmask);
extern uint32_t _set_basepri(uint32_t basepri);
extern uint32_t _set_control(uint32_t set_control);

// function to check if we are in priviledged mode
extern uint32_t _is_priv(void);

extern void _system_select_msp();
extern void _system_select_psp();

extern void _system_svc_call(uint32_t svc_number);



int add (int a, int b)
{
		return (a + b );  // should return 2*a
}

/* Note while chaning stack pointer on the fly only call void functions
 * if paramets needs to return, return by reference. 
 * */
void add_psp(uint32_t a, uint32_t b, uint32_t* result){
		*result = add(a,b); 
}


int main(){
		uint32_t var = 20; 
		uint32_t stuck = 100; 
		uint32_t ret;
		uint32_t priv; // Store state of execution mode
		uint32_t primask;
		uint32_t faultmask;
		uint32_t basepri;
		uint32_t control; 

		priv = _is_priv(); // should return priv = 1 as we are in priv mode
						   
		/* primask checks */
		primask = _get_primask();
		ret = _set_primask(0x1);
		primask = _get_primask();
		ret = _set_primask(0x0);

		/* faultmask checks */
		faultmask = _get_faultmask();
		ret = _set_faultmask(0x1);
		faultmask = _get_faultmask();
		ret = _set_faultmask(0x0);

		/* basepri checks */
		basepri = _get_basepri();
		ret = _set_basepri(0x2);
		basepri = _get_basepri();
		ret = _set_basepri(0x0);

		control = _get_control(); // should have bit 0 as 0 as it is priv mode.
								  //
		// select PSP: process stack pointer
		_system_select_psp();
		add_psp (0xFA00, 0x00DA, &var); // this function is executed using Process Stack pointer
		_system_select_msp();


/* -------------Unpriv-thread-entry--------------------------------------------- */

		_enter_unpriv();
		var = add (3, 4); // execute this in unprivileged thread mode

		priv = _is_priv(); // should return priv = 0 as we are in unpriv mode
						   //
		ret = _set_primask(0x1); // should return failure (0xFF) as sepcial reg can not be written from unpriv state
		ret = _set_faultmask(0x1); // should return failure (0xFF) as sepcial reg can not be written from unpriv state
		ret = _set_basepri(0x2); // should return failure (0xFF) as sepcial reg can not be written from unpriv state
		control = _get_control(); // should how bit 0 as set as it is unpriv mode

		_system_svc_call(0x0);

		// Enter into Priv thread mode using SVC
		_system_svc_call(0x2); 

		// now should be in Priv Thread mode

		while (stuck != 0)
		{
		;
		}
		return ret; 
}
