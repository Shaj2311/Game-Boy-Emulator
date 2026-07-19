#include "instructions.h"
#include "gb.h"

void nop()
{
	return;
}

void daa()
{
	uint8_t adjustment = 0;
	//if subtract flag (N) is set,
	if((gb.F & 0x40))
	{
		//if half carry flag (H) is set, add 6
		adjustment += 0x6 * ((gb.F & 0x20) != 0);

		//if carry flag (C) is set, add 60
		adjustment += 0x60 * ((gb.F & 0x10) != 0);

		//subtract adjustment from A
		gb.A -= adjustment;
	}
	else
	{
		if(gb.F & 0x20 || (gb.A & 0xF) > 0x9)
			adjustment += 0x6;

		if(gb.F & 0x10 || gb.A > 0x99)
		{
			adjustment += 0x60;
			//set carry flag
			gb.F |= 0x10;
		}
		else
			//unset carry flag
			gb.F &= 0xEF;

		gb.A += adjustment;
	}

	//update flags
	if(!gb.A)
		gb.F |= 0x80;
	else
		gb.F &= 0x7F;
	gb.F &= 0xDF;
}

void stop()
{
	//skip next dummy byte
	gb.PC++;
}
