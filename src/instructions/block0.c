#include "instructions.h"
#include "gb.h"

//hardcoded bit manip (reg A)
void rlca()
{
	//get MSB of A reg
	uint8_t msb = (gb.A & 0x80) >> 7;

	//rotate left
	gb.A = (gb.A << 1) | msb;

	//set carry flag to MSB and other flags to zero
	gb.F = msb ? 0x10 : 0x00;
}

void rrca()
{
	//get LSB of A reg
	uint8_t lsb = gb.A & 0x01;

	//rotate right
	gb.A = (gb.A >> 1) | (lsb << 7);

	//set carry flag to LSB and other flags to zero
	gb.F = lsb ? 0x10 : 0x00;
}

void rla()
{
	//get MSB
	uint8_t msb = (gb.A & 0x80) >> 7;

	//rotate left through carry
	gb.A = (gb.A << 1) | ((gb.F & 0x10) >> 4);

	//update carry
	gb.F = msb ? 0x10 : 0x00;
}

void rra()
{
	//get LSB
	uint8_t lsb = gb.A & 0x01;

	//rotate right through carry
	gb.A = (gb.A >> 1) | ((gb.F & 0x10) << 3);

	//update carry
	gb.F = lsb ? 0x10 : 0x00;
}

void cpl()
{
	//complement A reg
	gb.A = ~gb.A;

	//set flags
	gb.F |= 0x60;
}

void scf()
{
	//set carry flag
	gb.F |= 0x10;
}

void ccf()
{
	//complement carry flag
	gb.F ^= 0x10;

	//update other flags
	gb.F &= 0x9F;
}


// misc
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
