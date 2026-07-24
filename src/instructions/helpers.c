#include "instructions.h"
#include "gb.h"
#include <stdio.h>
#include <stdlib.h>


//helpers
uint16_t *get_r16(uint8_t r16)
{
	switch(r16)
	{
		case 0:
			return &gb.BC;
		case 1:
			return &gb.DE;
		case 2:
			return &gb.HL;
		case 3:
			return &gb.SP;
		default:
			puts("Invalid register code");
			exit(1);
	}
}

uint16_t *get_r16mem(uint8_t r16mem, int *offset)
{
	*offset = 0;
	switch(r16mem)
	{
		case 0:
			return &gb.BC;
		case 1:
			return &gb.DE;
		case 2:
			*offset = 1;
			return &gb.HL;
		case 3:
			*offset = -1;
			return &gb.HL;
		default:
			puts("Invalid register code");
			exit(1);
	}
}

uint8_t read_r8(uint8_t r8)
{
	switch(r8)
	{
		case 0:
			return gb.B;
		case 1:
			return gb.C;
		case 2:
			return gb.D;
		case 3:
			return gb.E;
		case 4:
			return gb.H;
		case 5:
			return gb.L;
		case 6:
			return mmu_read(gb.HL);
		case 7:
			return gb.A;
		default:
			puts("Invalid register code");
			exit(1);
	}
}

void write_r8(uint8_t r8, uint8_t val)
{
	switch(r8)
	{
		case 0:
			gb.B = val;
			break;
		case 1:
			gb.C = val;
			break;
		case 2:
			gb.D = val;
			break;
		case 3:
			gb.E = val;
			break;
		case 4:
			gb.H = val;
			break;
		case 5:
			gb.L = val;
			break;
		case 6:
			mmu_write(gb.HL, val);
			break;
		case 7:
			gb.A = val;
			break;
		default:
			puts("Invalid register code");
			exit(1);
	}
}

uint8_t is_cond_true(uint8_t cond)
{
	switch(cond)
	{
		case 0://nz
			return (!(gb.F & 0x80));
		case 1://z
			return (gb.F & 0x80);
		case 2://nc
			return (!(gb.F & 0x10));
		case 3://c
			return (gb.F & 0x10);
		default:
			puts("Invalid condition code");
			exit(1);
	}
}

uint16_t get_imm16()
{
	uint8_t low = mmu_read(gb.PC++);
	uint8_t high = mmu_read(gb.PC++);
	return (high << 8) | low;
}

uint16_t *get_r16stk(uint8_t r16)
{
	switch(r16)
	{
		case 0:
			return &gb.BC;
		case 1:
			return &gb.DE;
		case 2:
			return &gb.HL;
		case 3:
			return &gb.AF;
		default:
			puts("Invalid register code");
			exit(1);
	}
}
