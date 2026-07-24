#include "gb.h"
#include "instructions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define DBG_CARTRIDGE "roms/cpu_instrs.gb"

GameBoy gb;
const uint8_t bootROM[256] =
{
	0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
	0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
	0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
	0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
	0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
	0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
	0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
	0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
	0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
	0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
	0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
	0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
	0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};
char *rom;

int main()
{
	//boot game boy
	gb_boot();

	puts("Executing cartridge...");
	//FDE cycle
	while(1)
	{
		//check IME
		if(gb.IME_scheduled == 2)
		{
			//enable interrupts
			gb.IME = 1;
			gb.IME_scheduled = 0;
		}
		else if(gb.IME_scheduled == 1)
		{
			//one instruction delay
			gb.IME_scheduled = 2;
		}

		//get instruction
		uint8_t instruction = mmu_read(gb.PC++);

		//execute instruction
		gb_execute(instruction);
	}
}

void gb_boot()
{
	//reset CPU registers
	gb.AF = 0;
	gb.BC = 0;
	gb.DE = 0;
	gb.HL = 0;
	gb.SP = 0xFFFE;
	gb.PC = 0;

	////DEBUG: Skip boot ROM
	//gb.PC = 0x100;
	//gb.SP = 0xFFFE;
	//gb.A  = 0x01;
	//gb.F  = 0xB0;
	//gb.B  = 0x00;
	//gb.C  = 0x13;
	//gb.D  = 0x00;
	//gb.E  = 0xD8;
	//gb.H  = 0x01;
	//gb.L  = 0x4D;

	//reset boot ROM mapping control
	gb.sysbus[0xFF50] = 0;

	//load cartridge
	gb_load_cartridge(DBG_CARTRIDGE);

	//don't halt
	gb.halted = 0;

	//disable interrupts
	gb.IME = 0;
	//do not enable interrupts
	gb.IME_scheduled = 0;
}

void gb_load_cartridge(const char *cartridge)
{
	printf("Reading cartridge %s...\n", cartridge);

	//get cartridge
	FILE *romFile = fopen(cartridge, "rb");
	if(!romFile)
	{
		puts("Error reading cartridge");
		exit(1);
	}

	//check size of cartridge
	if(fseek(romFile, 0, SEEK_END) == -1)
	{
		puts("Error reading cartridge");
		exit(1);
	}
	long romSize = ftell(romFile);
	if(romSize == -1)
	{
		puts("Error reading cartridge");
		exit(1);
	}

	if(fseek(romFile, 0, SEEK_SET) == -1)
	{
		puts("Error reading cartridge");
		exit(1);
	}

	//read cartridge
	rom = malloc(romSize);
	size_t itemsRead = fread(rom, 1, romSize, romFile);
	if(itemsRead < (size_t)romSize)
	{
		puts("Error reading cartridge");
		exit(1);
	}
	if(fclose(romFile) != 0)
	{
		puts("Error reading cartridge");
		exit(1);
	}

	//load into memory (0x0000 - 0x7FFF)
	memcpy(gb.sysbus, rom, (romSize < 32768 ? romSize : 32768));

	puts("Cartridge loaded successfully");
}

void gb_execute(uint8_t instruction)
{
	//extract information
	uint8_t r8 = (instruction & 0x3F) >> 3;
	uint8_t r16 = (instruction & 0x3F) >> 4;
	uint8_t cond = (instruction & 0x1F) >> 3;

	//decode instruction
	//check 2 MSB's for block number
	switch(instruction >> 6)
	{
		case 0b00:
			//check 3 LSB's for instruction type
			switch(instruction & 0x07)
			{
				case 0b000:
					switch(r8)
					{
						case 0b000:
							nop();
							break;
						case 0b010:
							stop();
							break;
						case 0b011:
							jr_imm8();
							break;
						default:
							if(instruction & 0x20)
								jr_cond_imm8(cond);
							else
								gb_exit_invalid_opcode(instruction);
							break;
					}
					break;
				case 0b001:
					switch(instruction & 0x08)
					{
						case 0:
							ld_r16_imm16(r16);
							break;
						case 1:
							add_hl_r16(r16);
							break;
					}
					break;
				case 0b010:
					switch(instruction & 0x08)
					{
						case 0:
							ld_R16MEM_a(r16);
							break;
						case 1:
							ld_a_R16MEM(r16);
							break;
					}
					break;
				case 0b011:
					switch(instruction & 0x08)
					{
						case 0:
							inc_r16(r16);
							break;
						case 1:
							dec_r16(r16);
							break;
					}
					break;
				case 0b100:
					inc_r8(r8);
					break;
				case 0b101:
					dec_r8(r8);
					break;
				case 0b110:
					ld_r8_imm8(r8);
					break;
				case 0b111:
					switch(r8)
					{
						case 0: rlca(); break;
						case 1: rrca(); break;
						case 2: rla(); break;
						case 3: rra(); break;
						case 4: daa(); break;
						case 5: cpl(); break;
						case 6: scf(); break;
						case 7: ccf(); break;
					}
			}
			break;
		case 0b01:
			switch(instruction & 0x3F)
			{
				case 0b110110:
					halt();
					break;
				default:
					ld_r8_r8(r8, instruction & 0x07);
					break;
			}
			break;
		case 0b10:
			switch(r8)
			{
				case 0:
					add_a_r8(r8);
					break;
				case 1:
					adc_a_r8(r8);
					break;
				case 2:
					sbc_a_r8(r8);
					break;
				case 3:
					sbc_a_r8(r8);
					break;
				case 4:
					and_a_r8(r8);
					break;
				case 5:
					xor_a_r8(r8);
					break;
				case 6:
					or_a_r8(r8);
					break;
				case 7:
					cp_a_r8(r8);
					break;
			}
			break;
		case 0b11:
			switch(instruction & 0x07)
			{
				case 0b000:
					switch(r8)
					{
						case 0b100:
							ldh_IMM8_a();
							break;
						case 0b101:
							add_sp_imm8();
							break;
						case 0b110:
							ldh_a_IMM8();
							break;
						case 0b111:
							ld_hl_spPLUSimm8();
							break;
						default:
							if((instruction & 0x20) == 0)
								ret_cond(cond);
							else
								gb_exit_invalid_opcode(instruction);
							break;
					}
					break;
				case 0b001:
					switch(r8)
					{
						case 0b001:
							ret();
							break;
						case 0b011:
							reti();
							break;
						case 0b101:
							jp_hl();
							break;
						case 0b111:
							ld_sp_hl();
							break;
						default:
							if((instruction & 0x08) == 0)
								pop_r16stk(r16);
							else
								gb_exit_invalid_opcode(instruction);
							break;
					}
					break;
				case 0b010:
					switch(r8)
					{
						case 0b100:
							ldh_C_a();
							break;
						case 0b101:
							ld_IMM16_a();
							break;
						case 0b110:
							ldh_a_C();
							break;
						case 0b111:
							ld_a_IMM16();
							break;
						default:
							if((instruction & 0x20) == 0)
								jp_cond_imm16(cond);
							else
								gb_exit_invalid_opcode(instruction);
							break;
					}
					break;
				case 0b011:
					switch(r8)
					{
						case 0b000:
							jp_imm16();
							break;
						case 0b001:
							//prefix
							//get next instruction
							instruction = mmu_read(++gb.PC);
							//parse params
							r8 = instruction & 0x07;
							uint8_t b3 = (instruction >> 3) & 0x07;

							//check two MSBs
							switch(instruction >> 6)
							{
								case 0:
									//check middle 3 bits
									switch(instruction >> 3)
									{
										case 0:
											rlc_r8(r8);
											break;
										case 1:
											rrc_r8(r8);
											break;
										case 2:
											rl_r8(r8);
											break;
										case 3:
											rr_r8(r8);
											break;
										case 4:
											sla_r8(r8);
											break;
										case 5:
											sra_r8(r8);
											break;
										case 6:
											swap_r8(r8);
											break;
										case 7:
											srl_r8(r8);
											break;
									}
									break;
								case 1:
									bit_b3_r8(b3, r8);
									break;
								case 2:
									res_b3_r8(b3, r8);
									break;
								case 3:
									set_b3_r8(b3, r8);
									break;
							}
							break;
						case 0b110:
							di();
							break;
						case 0b111:
							ei();
							break;
						default:
							gb_exit_invalid_opcode(instruction);
					}
					break;
				case 0b100:
					call_cond_imm16(cond);
					break;
				case 0b101:
					if(r8 == 0b001)
						call_imm16();
					else if((instruction & 0x08) == 0)
						push_r16stk(r16);
					else
						gb_exit_invalid_opcode(instruction);
					break;
				case 0b110:
					switch(r8)
					{
						case 0:
							add_a_imm8();
							break;
						case 1:
							adc_a_imm8();
							break;
						case 2:
							sub_a_imm8();
							break;
						case 3:
							sbc_a_imm8();
							break;
						case 4:
							and_a_imm8();
							break;
						case 5:
							xor_a_imm8();
							break;
						case 6:
							or_a_imm8();
							break;
						case 7:
							cp_a_imm8();
							break;
					}
					break;
				case 0b111:
					rst_tgt3(r8);
					break;
			}
			break;
	}
}

uint8_t mmu_read(uint16_t addr)
{
	if(addr < 0x0100)
	{
		//check boot ROM switch
		if(gb.sysbus[0xFF50])
		{
			//read from game cartridge
			return gb.sysbus[addr];
		}
		else
		{
			//read from boot ROM
			return bootROM[addr];
		}
	}

	return gb.sysbus[addr];
}

void mmu_write(uint16_t addr, uint8_t val)
{
	//writing to boot ROM switch
	if(addr == 0xFF50)
	{
		if(!gb.sysbus[0xFF50])
			gb.sysbus[addr] = val;
	}

	//writing to WRAM (write to echo RAM as well)
	else if(addr >= 0xC000 && addr <= 0xDDFF)
	{
		//write to WRAM
		gb.sysbus[addr] = val;
		//also write to echo RAM
		gb.sysbus[addr + 0x2000] = val;
	}

	//attempting to write to ROM
	else if(addr <= 0x7FFF)
		return;

	//writing normally
	else
		gb.sysbus[addr] = val;
}

void gb_exit_invalid_opcode(uint8_t instruction)
{
	printf("Invalid opcode: 0x%02x\n", instruction);
	exit(1);
}
