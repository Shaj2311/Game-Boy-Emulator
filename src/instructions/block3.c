#include "instructions.h"
#include "gb.h"
#include <stdio.h>
#include <stdlib.h>

void add_a_imm8()
{
	//add
	uint8_t oldA = gb.A;
	uint8_t addVal = mmu_read(gb.PC++);
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

void adc_a_imm8()
{
	//add
	uint8_t oldA = gb.A;
	uint8_t carry = (gb.F & 0x10) >> 4;
	uint8_t addVal = mmu_read(gb.PC++);
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

void sub_a_imm8()
{
	//subtract
	uint8_t oldA = gb.A;
	uint8_t subVal = mmu_read(gb.PC++);
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

void sbc_a_imm8()
{
	//subtract
	uint8_t oldA = gb.A;
	uint8_t subVal = mmu_read(gb.PC++);
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

void and_a_imm8()
{
	//and
	gb.A &= mmu_read(gb.PC++);

	//set flags
	gb.F = 0x20; //reset all flags, H = 1
	if(!gb.A)
		gb.F |= 0x80; //Z = 1
}

void xor_a_imm8()
{
	//xor
	gb.A ^= mmu_read(gb.PC++);

	//set flags
	gb.F = 0; //reset all flags
	if(!gb.A)
		gb.F |= 0x80; //Z = 1
}

void or_a_imm8()
{
	//or
	gb.A |= mmu_read(gb.PC++);

	//set flags
	gb.F = 0; //reset all flags
	if(!gb.A)
		gb.F |= 0x80; //Z = 1
}

void cp_a_imm8()
{
	//subtract
	uint8_t oldA = gb.A;
	uint8_t subVal = mmu_read(gb.PC++);
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
void ret_cond(uint8_t cond)
{
	if(is_cond_true(cond))
	{
		ret();
	}
}

void ret()
{
	//return from subroutine
	//pop PC
	uint8_t low = mmu_read(gb.SP++);
	uint8_t high = mmu_read(gb.SP++);
	gb.PC = (high << 8) | low;
}

void reti()
{
	//return from subroutine
	ret();
	//enable interrupts
	gb.IME = 1;
}

void jp_cond_imm16(uint8_t cond)
{
	//get imm16
	uint16_t imm16 = get_imm16();

	if(is_cond_true(cond))
	{
		//jump to imm16
		gb.PC = imm16;
	}
}

void jp_imm16()
{
	//jump to imm16
	gb.PC = get_imm16();
}

void jp_hl()
{
	gb.PC = gb.HL;
}

void call_cond_imm16(uint8_t cond)
{
	//get imm16
	uint16_t imm16 = get_imm16();

	//check condition
	if(is_cond_true(cond))
	{
		//call

		//push address of instruction onto stack
		mmu_write(--gb.SP, gb.PC >> 8);
		mmu_write(--gb.SP, gb.PC & 0xFF);

		//jp imm16
		gb.PC = imm16;
	}
}

void call_imm16()
{
	//get imm16
	uint16_t imm16 = get_imm16();

	//push address of instruction onto stack
	mmu_write(--gb.SP, gb.PC >> 8);
	mmu_write(--gb.SP, gb.PC & 0xFF);

	//jp imm16
	gb.PC = imm16;
}

void rst_tgt3(uint8_t tgt3)
{
	//push address of instruction onto stack
	mmu_write(--gb.SP, gb.PC >> 8);
	mmu_write(--gb.SP, gb.PC & 0xFF);

	//jump to target
	gb.PC = tgt3 * 8;
}

void pop_r16stk(uint16_t r16stk)
{
	//get register
	uint16_t *reg = get_r16stk(r16stk);

	//pop from stack to register
	uint8_t low = mmu_read(gb.SP++);
	uint8_t high = mmu_read(gb.SP++);
	*reg = (high << 8) | low;

	//if register is AF, mask out last nibble (lower nibble of Flags)
	if(reg == &gb.AF)
		*reg &= 0xFFF0;
}

void push_r16stk(uint16_t r16stk)
{
	//get register
	uint16_t *reg = get_r16stk(r16stk);

	//push register value onto stack
	mmu_write(--gb.SP, *reg >> 8);
	mmu_write(--gb.SP, *reg & 0xFF);
}

void ldh_C_a()
{
	//[0xFF00 + C] = A
	mmu_write(0xFF00 + gb.C, gb.A);
}

void ldh_IMM8_a()
{
	//[0xFF00 + imm8] = A
	//get imm8
	uint8_t imm8 = mmu_read(gb.PC++);
	//write to high ram
	mmu_write(0xFF00 + imm8, gb.A);
}

void ld_IMM16_a()
{
	//get imm16
	uint16_t imm16 = get_imm16();
	mmu_write(imm16, gb.A);
}

void ldh_a_C()
{
	//A = [0xFF00 + C]
	gb.A = mmu_read(0xFF00 + gb.C);
}

void ldh_a_IMM8()
{
	//A = [0xFF00 + imm8]
	uint8_t imm8 = mmu_read(gb.PC++);
	gb.A = mmu_read(0xFF00 + imm8);
}

void ld_a_IMM16()
{
	gb.A = mmu_read(get_imm16());
}

void add_sp_imm8()
{
	uint16_t oldSP = gb.SP;
	int8_t offset = mmu_read(gb.PC++);
	gb.SP += offset;

	//reset flags
	gb.F = 0;
	if((oldSP & 0x000F) + ((uint8_t)offset & 0x0F) > 0x0F)
		gb.F |= 0x20; //H = 1
	if((oldSP & 0x00FF) + (uint8_t)offset > 0x00FF)
		gb.F |= 0x10; //C = 1
}

void ld_hl_spPLUSimm8()
{
	uint16_t oldSP = gb.SP;
	int8_t offset = mmu_read(gb.PC++);
	gb.HL = oldSP + offset;

	//reset flags
	gb.F = 0;
	if((oldSP & 0x000F) + ((uint8_t)offset & 0x0F) > 0x0F)
		gb.F |= 0x20; //H = 1
	if((oldSP & 0x00FF) + (uint8_t)offset > 0x00FF)
		gb.F |= 0x10; //C = 1
}

void ld_sp_hl()
{
	gb.SP = gb.HL;
}

void di()
{
	gb.IME = gb.IME_scheduled = 0;
}

void ei()
{
	gb.IME_scheduled = 1;
}

void rlc_r8(uint8_t r8)
{
	//get r8
	uint8_t regval = read_r8(r8);
	//get msb
	uint8_t msb = regval >> 7;
	//shift left
	regval <<= 1;
	//wrap msb to lsb
	regval |= msb;
	//write to register
	write_r8(r8, regval);

	//reset flags
	gb.F = 0;
	if(!regval)
		gb.F |= 0x80; //Z = 1
	gb.F |= 0x10 * msb; //C = MSB
}

void rrc_r8(uint8_t r8)
{
	//get r8
	uint8_t regval = read_r8(r8);
	//get lsb
	uint8_t lsb = regval & 0x01;
	//shift right
	regval >>= 1;
	//wrap lsb to msb
	regval |= lsb << 7;
	//write to register
	write_r8(r8, regval);

	//reset flags
	gb.F = 0;
	if(!regval)
		gb.F |= 0x80; //Z = 1
	gb.F |= 0x10 * lsb; //C = MSB
}

void rl_r8(uint8_t r8)
{
	uint8_t carry = (gb.F & 0x10) >> 4;
	uint8_t regval = read_r8(r8);
	uint8_t msb = regval >> 7;

	//shift left
	regval <<= 1;
	//set lsb to carry
	regval |= carry;
	//write to register
	write_r8(r8, regval);

	//reset flags
	gb.F = 0;
	if(!regval)
		gb.F |= 0x80; //Z = 1
	gb.F |= 0x10 * msb; //C = MSB
}

void rr_r8(uint8_t r8)
{
	uint8_t carry = (gb.F & 0x10) >> 4;
	uint8_t regval = read_r8(r8);
	uint8_t lsb = regval & 0x01;

	//shift right
	regval >>= 1;
	//set msb to carry
	regval |= 0x80 * carry;
	//write to register
	write_r8(r8, regval);

	//reset flags
	gb.F = 0;
	if(!regval)
		gb.F |= 0x80; //Z = 1
	gb.F |= 0x10 * lsb; //C = MSB
}

void sla_r8(uint8_t r8)
{
	uint8_t regval = read_r8(r8);
	uint8_t msb = regval >> 7;

	//shift left
	regval <<= 1;
	//write to register
	write_r8(r8, regval);

	//reset flags
	gb.F = 0;
	if(!regval)
		gb.F |= 0x80; //Z = 1
	gb.F |= 0x10 * msb; //C = MSB
}

void sra_r8(uint8_t r8)
{
	uint8_t regval = read_r8(r8);
	uint8_t msb = regval >> 7;
	uint8_t lsb = regval & 0x01;

	//shift right
	regval >>= 1;
	//restore MSB
	regval |= 0x80 * msb;
	//write to register
	write_r8(r8, regval);

	//reset flags
	gb.F = 0;
	if(!regval)
		gb.F |= 0x80; //Z = 1
	gb.F |= 0x10 * lsb; //C = LSB
}

void swap_r8(uint8_t r8)
{
	uint8_t regval = read_r8(r8);
	uint8_t high = regval >> 4;
	uint8_t low = regval & 0x0F;
	regval = (low << 4) | high;
	write_r8(r8, regval);

	//reset flags
	gb.F = 0;
	if(!regval)
		gb.F |= 0x80; //Z = 1
}

void srl_r8(uint8_t r8)
{
	uint8_t regval = read_r8(r8);
	uint8_t lsb = regval & 0x01;

	//shift right
	regval >>= 1;
	//write to register
	write_r8(r8, regval);

	//reset flags
	gb.F = 0;
	if(!regval)
		gb.F |= 0x80; //Z = 1
	gb.F |= 0x10 * lsb; //C = LSB
}

void bit_b3_r8(uint8_t b3, uint8_t r8)
{
	//reset Z,N,H
	gb.F &= 0x10;
	if(!((read_r8(r8) >> b3) & 0x01))
		gb.F |= 0x80; //Z = 1
	gb.F |= 0x20; // H = 1
}

void res_b3_r8(uint8_t b3, uint8_t r8)
{
	uint8_t regval = read_r8(r8);
	regval &= (~(0x01 << b3));
	write_r8(r8, regval);
}

void set_b3_r8(uint8_t b3, uint8_t r8)
{
	uint8_t regval = read_r8(r8);
	regval |= (0x01 << b3);
	write_r8(r8, regval);
}
