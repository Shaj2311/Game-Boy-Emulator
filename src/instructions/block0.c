#include "instructions.h"
#include "gb.h"
#include <stdio.h>
#include <stdlib.h>

//load instructions
void ld_r16_imm16(uint8_t r16)
{
	//get r16
	uint16_t *reg = get_r16(r16);

	//get imm16 (little Endian)
	uint8_t lowByte = mmu_read(gb.PC++);
	uint8_t highByte = mmu_read(gb.PC++);
	uint16_t imm16 = (highByte << 8) | lowByte;

	//load
	*reg = imm16;
}

void ld_R16MEM_a(uint8_t r16mem)
{
	//get r16mem
	int offset;
	uint16_t *reg = get_r16mem(r16mem, &offset);

	//load value of a into system bus
	mmu_write(*reg, gb.A);

	//increment or decrement HL if needed
	(*reg) += offset;
}

void ld_a_R16MEM(uint8_t r16mem)
{
	//get r16mem
	int offset;
	uint16_t *reg = get_r16mem(r16mem, &offset);

	//load value into a
	gb.A = mmu_read(*reg);

	//apply offset (inc/dec) if needed
	(*reg) += offset;
}

void ld_IMM16_sp()
{
	//get imm16
	uint8_t lowByte = mmu_read(gb.PC++);
	uint8_t highByte = mmu_read(gb.PC++);
	uint16_t imm16 = (highByte << 8) | lowByte;

	//load value of sp into system bus
	mmu_write(imm16, gb.SP & 0x00FF);
	mmu_write(imm16 + 1, gb.SP >> 8);
}

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
