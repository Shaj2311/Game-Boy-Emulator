#include "instructions.h"
#include "gb.h"

uint8_t ld_r8_r8(uint8_t r8_1, uint8_t r8_2)
{
	write_r8(r8_1, read_r8(r8_2));

	if(r8_1 == 6 || r8_2 == 6)
		return 8;
	return 4;
}

void halt()
{
	if(!gb.IME && (mmu_read(0xFFFF) & mmu_read(0xFF0F) & 0x1F))
	{
		gb.halted = 0;
		gb.PC--;
	}
	else
		gb.halted = 1;
}
