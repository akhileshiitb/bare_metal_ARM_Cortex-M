#include<stdio.h>
#include<stdint.h>

extern void _enter_unpriv();
extern uint32_t _get_primask();
extern uint32_t _get_faultmask();
extern uint32_t _get_basepri();

extern uint32_t _set_primask(uint32_t primask);
extern uint32_t _set_faultmask(uint32_t faultmask);
extern uint32_t _set_basepri(uint32_t basepri);

// function to check if we are in priviledged mode
extern uint32_t _is_priv(void);

int add (int a, int b)
{
		return a + b; 
}


int main(){
		int var = 20; 
		uint32_t ret;
		uint32_t priv; // Store state of execution mode
		uint32_t primask;
		uint32_t faultmask;
		uint32_t basepri;

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


/* -------------Unpriv-thread-entry--------------------------------------------- */

		_enter_unpriv();
		var = add (3, 4); // execute this in unprivileged thread mode

		priv = _is_priv(); // should return priv = 0 as we are in unpriv mode
						   //
		ret = _set_primask(0x1); // should return failure (0xFF) as sepcial reg can not be written from unpriv state
		ret = _set_faultmask(0x1); // should return failure (0xFF) as sepcial reg can not be written from unpriv state
		ret = _set_basepri(0x2); // should return failure (0xFF) as sepcial reg can not be written from unpriv state

		while (var != 10)
		{
		;
		}
		return ret; 
}
