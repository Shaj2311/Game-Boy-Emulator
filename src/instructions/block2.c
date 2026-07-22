#include "instructions.h"
#include "gb.h"

void add_a_r8(uint8_t r8)
{
	//add
	uint8_t oldA = gb.A;
	uint8_t addVal = read_r8(r8);
	gb.A += addVal;

	//set flags
	gb.F = 0; //reset all flags
	if(!gb.A)
		gb.F |= 0x80; //Z = 1
	if((oldA & 0x0F) + (addVal & 0x0F) > 0x0F)
		gb.F |= 0x20; //H = 1
	if(((uint16_t)oldA + (uint16_t)addVal) > 0xFF)
		gb.F |= 0x10;
}

void adc_a_r8(uint8_t r8)
{
	//add
	uint8_t oldA = gb.A;
	uint8_t carry = (gb.F & 0x10) >> 4;
	uint8_t addVal = read_r8(r8);
	gb.A = oldA + addVal + carry;

	//set flags
	gb.F = 0; //reset all flags
	if(!gb.A)
		gb.F |= 0x80; //Z = 1
	if((oldA & 0x0F) + (addVal & 0x0F) + carry > 0x0F)
		gb.F |= 0x20; //H = 1
	if((uint16_t)oldA + (uint16_t)(addVal) + carry > 0xFF)
		gb.F |= 0x10;
}

void sub_a_r8(uint8_t r8)
{
	//subtract
	uint8_t oldA = gb.A;
	uint8_t subVal = read_r8(r8);
	gb.A -= subVal;

	//set flags
	gb.F = 0x40; //reset all flags, N = 1
	if(!gb.A)
		gb.F |= 0x80; //Z = 1
	if((oldA & 0x0F) < (subVal & 0x0F))
		gb.F |= 0x20; //H = 1
	if(subVal > oldA)
		gb.F |= 0x10; //C = 1
}

void sbc_a_r8(uint8_t r8)
{
	//subtract
	uint8_t oldA = gb.A;
	uint8_t subVal = read_r8(r8);
	uint8_t carry = (gb.F >> 4) & 0x01;
	gb.A = oldA - subVal - carry;

	//set flags
	gb.F = 0x40; //reset all flags, N = 1
	if(!gb.A)
		gb.F |= 0x80;
	if((oldA & 0x0F) < (subVal & 0x0F) + carry)
		gb.F |= 0x20; //H = 1
	if((uint16_t)subVal + carry > oldA)
		gb.F |= 0x10; //C = 1
}

void and_a_r8(uint8_t r8)
{
	//and
	gb.A &= read_r8(r8);

	//set flags
	gb.F = 0x20; //reset all flags, H = 1
	if(!gb.A)
		gb.F |= 0x80; //Z = 1
}

void xor_a_r8(uint8_t r8)
{
	//xor
	gb.A ^= read_r8(r8);

	//set flags
	gb.F = 0; //reset all flags
	if(!gb.A)
		gb.F |= 0x80; //Z = 1
}

void or_a_r8(uint8_t r8)
{
	//or
	gb.A |= read_r8(r8);

	//set flags
	gb.F = 0; //reset all flags
	if(!gb.A)
		gb.F |= 0x80; //Z = 1

}

void cp_a_r8(uint8_t r8)
{
	//subtract
	uint8_t oldA = gb.A;
	uint8_t subVal = read_r8(r8);
	uint8_t result = gb.A - subVal;

	//set flags
	gb.F = 0x40; //reset all flags, N = 1
	if(!result)
		gb.F |= 0x80; //Z = 1
	if((oldA & 0x0F) < (subVal & 0x0F))
		gb.F |= 0x20; //H = 1
	if(subVal > oldA)
		gb.F |= 0x10; //C = 1
}
