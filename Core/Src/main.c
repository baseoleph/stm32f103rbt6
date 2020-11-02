
#include "main.h"

int main(void)
{

	*(unsigned long*)0x40021018 |= 0x4;

	volatile unsigned long i=0;
	  i++; i++; i++;
	  i=0;

	 *(unsigned long*)0x400108000 |= 0x20;
	while (1)
	{

	}
}
