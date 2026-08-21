#include "instructions.h"
#include "gb.h"

void ld_r8_r8(uint8_t r8_1, uint8_t r8_2)
{
	write_r8(r8_1, read_r8(r8_2));
}

void halt()
{
	if(!gb.IME && (gb.sysbus[0xFFFF] & gb.sysbus[0xFF0F] & 0x1F))
	{
		gb.halted = 0;
	}
	else
		gb.halted = 1;
}
