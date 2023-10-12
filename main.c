#include<stdio.h>
#include<stdint.h>

extern void _enter_unpriv();

int add (int a, int b)
{
		return a + b; 
}


int main(){
		int var = 20; 

		_enter_unpriv();
		var = add (3, 4); // execute this in unprivileged thread mode

		while (var != 10)
		{
		;
		}
		return var; 
}
