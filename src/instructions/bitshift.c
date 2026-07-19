#include "instructions.h"
#include "gb.h"

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
