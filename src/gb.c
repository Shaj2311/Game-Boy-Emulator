#include "gb.h"
#include "instructions.h"
#include "timers.h"
#include "mmu.h"
#include "addr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define DBG_CARTRIDGE "roms/games/Street Fighter II (USA, Europe) (Rev 1) (SGB Enhanced).gb"

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
const uint32_t RGBA[4] = {
	0x9BBC0FFF,
	0x8BAC0FFF,
	0x306230FF,
	0x0F380FFF
};

void gb_boot()
{
	//reset CPU registers
	gb.AF = 0;
	gb.BC = 0;
	gb.DE = 0;
	gb.HL = 0;
	gb.SP = 0xFFFE;
	gb.PC = 0;

	//reset boot ROM mapping control
	gb.sysbus[0xFF50] = 0;

	//read from ROM bank 1
	gb.currRomBank = 1;

	//disable RAM access
	gb.ramEnable = 0;

	//reset system clock
	gb.clock = 0;

	//load cartridge
	gb_load_cartridge(DBG_CARTRIDGE);

	//initialize cartridge RAM
	gb_init_cartridge_ram();

	//don't halt
	gb.halted = 0;

	//disable interrupts
	gb.IME = 0;
	//do not enable interrupts
	gb.IME_scheduled = 0;

	//do not reset TIMA
	gb.TIMA_scheduled = 0;

	//initialize old STAT interrupt state
	gb.STAT_old_int = 0;

	gb.ppu_cycles = 0;
	//start with OAM search
	gb.ppu_mode = PPU_MODE_OAM_SEARCH;
	gb.sysbus[STAT_ADDR] = (gb.sysbus[STAT_ADDR] & 0xFC) | PPU_MODE_OAM_SEARCH;

	//clean framebuffer
	uint32_t whiteRGBA = ppu_lookup_RGBA(0, gb.sysbus[OBP0_ADDR]);
	for(int i = 0; i < 144 * 160; i++)
		gb.frameBuffer[i] = whiteRGBA;
	//no lines rendered yet
	gb.windowLinesRendered = 0;

	//reset joypad inputs
	gb.sysbus[JOYP_ADDR] = 0xCF;
	gb.joypadInputs = 0xFF;
}

void gb_init_cartridge_ram()
{
	gb.cartridgeRAM = 0;
	gb.cartridgeRamSize = 0;

	//read RAM size from ROM
	uint8_t ramSizeCode = gb.rom[0x0149];
	switch(ramSizeCode)
	{
		case 1:
			gb.cartridgeRamSize = 2048;
			break;
		case 2:
			gb.cartridgeRamSize = 8192;
			break;
		case 3:
			gb.cartridgeRamSize = 32768;
			break;
		case 4:
			gb.cartridgeRamSize = 131072;
			break;
		case 5:
			gb.cartridgeRamSize = 65536;
			break;
	}

	//initialize cartridge RAM
	if(gb.cartridgeRamSize > 0)
	{
		gb.cartridgeRAM = calloc(1, gb.cartridgeRamSize);
		if(!gb.cartridgeRAM)
		{
			puts("Error allocating cartridge RAM");
			exit(1);
		}
	}
}
void gb_load_cartridge(const char *cartridge)
{
	printf("Reading cartridge %s\n", cartridge);

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
	gb.romSize = ftell(romFile);
	if(gb.romSize == -1)
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
	gb.rom = malloc(gb.romSize);
	size_t itemsRead = fread(gb.rom, 1, gb.romSize, romFile);
	if(itemsRead < (size_t)gb.romSize)
	{
		puts("Error reading cartridge");
		exit(1);
	}
	if(fclose(romFile) != 0)
	{
		puts("Error reading cartridge");
		exit(1);
	}

	//load ROM bank 0 into memory (0x0000 - 0x3FFF)
	memcpy(gb.sysbus, gb.rom, (gb.romSize < 0x4000 ? gb.romSize : 0x4000));

	puts("Cartridge loaded successfully");

	//check ROM's MBC type
	uint8_t romType = gb.rom[0x0147];
	if(romType == 0x00 || romType == 0x08 || romType == 0x09)
		gb.mbcType = MBC_NONE;
	else if(romType >= 0x01 && romType <= 0x03)
		gb.mbcType = MBC_1;
	else if(romType >= 0x0F && romType <= 0x13)
		gb.mbcType = MBC_3;
	else if(romType >= 0x19 && romType <= 0x1E)
		gb.mbcType = MBC_5;
	else //fallback in case of unknown MBC type
		gb.mbcType = MBC_NONE;
}

void gb_service_interrupts()
{
	//get IE and IF
	uint8_t IE = gb.sysbus[0xFFFF];
	uint8_t IF = gb.sysbus[0xFF0F];

	//check unhalt condition
	if(gb.halted && (IE & IF & 0X1F))
		gb.halted = 0;

	//check master interrupt enable
	if(!gb.IME)
		return;

	//check each interrupt enable
	for(int i = 0; i <= 4; i++)
	{
		//skip if interrupt not enabled
		if(!((IE >> i) & 0x01))
			continue;

		//skip if interrupt not requested
		if(!((IF >> i) & 0x01))
			continue;

		//get address of service routine
		uint16_t targetAddr = 0x40 + (8 * i);

		//internal M-cycle tick 1
		for(int i = 0; i < 4; i++)
		{
			gb_timer_tick();
			ppu_timer_tick();
		}

		//push current address onto stack
		mmu_write(--gb.SP, gb.PC >> 8);
		mmu_write(--gb.SP, gb.PC & 0xFF);

		//internal M-cycle tick 2
		for(int i = 0; i < 4; i++)
		{
			gb_timer_tick();
			ppu_timer_tick();
		}

		//internal M-cycle tick 3
		for(int i = 0; i < 4; i++)
		{
			gb_timer_tick();
			ppu_timer_tick();
		}

		//jump to service routine
		gb.PC = targetAddr;

		//reset IF bit
		gb.sysbus[0xFF0F] &= ~(0x01 << i);

		//disable interrupts
		gb.IME = 0;

		//don't service any other interrupts
		return;
	}
}

void gb_execute(uint8_t instruction)
{
	//extract information
	uint8_t r8 = (instruction >> 3) & 0x07;
	uint8_t r16 = (instruction >> 4) & 0x03;
	uint8_t cond = (instruction >> 3) & 0x03;
	uint8_t srcReg = instruction & 0x07;

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
							nop(); break;
						case 0b001:
							ld_IMM16_sp(); break;
						case 0b010:
							stop(); break;
						case 0b011:
							jr_imm8(); break;
						default:
							if(instruction & 0x20)
								jr_cond_imm8(cond);
							else
								gb_exit_invalid_opcode(instruction);
							break;
					}
					break;
				case 0b001:
					switch((instruction >> 3) & 0x01)
					{
						case 0:
							ld_r16_imm16(r16); break;
						case 1:
							add_hl_r16(r16); break;
					}
					break;
				case 0b010:
					switch((instruction >> 3) & 0x01)
					{
						case 0:
							ld_R16MEM_a(r16); break;
						case 1:
							ld_a_R16MEM(r16); break;
					}
					break;
				case 0b011:
					switch((instruction >> 3) & 0x01)
					{
						case 0:
							inc_r16(r16); break;
						case 1:
							dec_r16(r16); break;
					}
					break;
				case 0b100:
					inc_r8(r8); break;
				case 0b101:
					dec_r8(r8); break;
				case 0b110:
					ld_r8_imm8(r8); break;
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
					halt(); break;
				default:
					ld_r8_r8(r8, instruction & 0x07); break;
			}
			break;
		case 0b10:
			switch(r8)
			{
				case 0:
					add_a_r8(srcReg); break;
				case 1:
					adc_a_r8(srcReg); break;
				case 2:
					sub_a_r8(srcReg); break;
				case 3:
					sbc_a_r8(srcReg); break;
				case 4:
					and_a_r8(srcReg); break;
				case 5:
					xor_a_r8(srcReg); break;
				case 6:
					or_a_r8(srcReg); break;
				case 7:
					cp_a_r8(srcReg); break;
			}
			break;
		case 0b11:
			switch(instruction & 0x07)
			{
				case 0b000:
					switch(r8)
					{
						case 0b100:
							ldh_IMM8_a(); break;
						case 0b101:
							add_sp_imm8(); break;
						case 0b110:
							ldh_a_IMM8(); break;
						case 0b111:
							ld_hl_spPLUSimm8(); break;
						default:
							ret_cond(cond); break;
					}
					break;
				case 0b001:
					switch(r8)
					{
						case 0b001:
							ret(); break;
						case 0b011:
							reti(); break;
						case 0b101:
							jp_hl(); break;
						case 0b111:
							ld_sp_hl(); break;
						default:
							pop_r16stk(r16); break;
					}
					break;
				case 0b010:
					switch(r8)
					{
						case 0b100:
							ldh_C_a(); break;
						case 0b101:
							ld_IMM16_a(); break;
						case 0b110:
							ldh_a_C(); break;
						case 0b111:
							ld_a_IMM16(); break;
						default:
							jp_cond_imm16(cond); break;
					}
					break;
				case 0b011:
					switch(r8)
					{
						case 0b000:
							jp_imm16(); break;
						case 0b001:
							//prefix
							//get next instruction
							instruction = mmu_read(gb.PC++);
							//parse params
							r8 = instruction & 0x07;
							uint8_t b3 = (instruction >> 3) & 0x07;

							//check two MSBs
							switch(instruction >> 6)
							{
								case 0:
									//check middle 3 bits
									switch(b3)
									{
										case 0:
											rlc_r8(r8); break;
										case 1:
											rrc_r8(r8); break;
										case 2:
											rl_r8(r8); break;
										case 3:
											rr_r8(r8); break;
										case 4:
											sla_r8(r8); break;
										case 5:
											sra_r8(r8); break;
										case 6:
											swap_r8(r8); break;
										case 7:
											srl_r8(r8); break;
									}
									break;
								case 1:
									bit_b3_r8(b3, r8); break;
								case 2:
									res_b3_r8(b3, r8); break;
								case 3:
									set_b3_r8(b3, r8); break;
							}
							break;
						case 0b110:
							di(); break;
						case 0b111:
							ei(); break;
						default:
							gb_exit_invalid_opcode(instruction);
					}
					break;
				case 0b100:
					call_cond_imm16(cond); break;
				case 0b101:
					if(r8 == 0b001)
						call_imm16();
					else if(((instruction >> 3) & 0x01) == 0)
						push_r16stk(r16);
					else
						gb_exit_invalid_opcode(instruction);
					break;
				case 0b110:
					switch(r8)
					{
						case 0:
							add_a_imm8(); break;
						case 1:
							adc_a_imm8(); break;
						case 2:
							sub_a_imm8(); break;
						case 3:
							sbc_a_imm8(); break;
						case 4:
							and_a_imm8(); break;
						case 5:
							xor_a_imm8(); break;
						case 6:
							or_a_imm8(); break;
						case 7:
							cp_a_imm8(); break;
					}
					break;
				case 0b111:
					rst_tgt3(r8); break;
			}
			break;
	}
}

void gb_timer_tick()
{
	//update system clock
	uint16_t oldClock = gb.clock;
	gb.clock++; //tick by 1 cycle
	uint16_t newClock = gb.clock;

	//update DIV (high byte of system clock)
	gb.sysbus[0xFF04] = newClock >> 8;

	//update TIMA
	//check TIMA update scheduled (check transition 1->0)
	if(gb.TIMA_scheduled)
	{
		gb.TIMA_scheduled--;
		if(gb.TIMA_scheduled == 0)
			gb.sysbus[0xFF05] = gb.sysbus[0xFF06]; //reset to TMA
	}

	//check TAC
	uint8_t TAC = gb.sysbus[0xFF07];
	uint8_t clockSelect = TAC & 3;
	//if TIMA increment is enabled,
	if(TAC & 0x04)
	{
		uint8_t TIMA = gb.sysbus[0xFF05];
		uint16_t bitmask;
		//increment TIMA depending on clock select
		switch(clockSelect)
		{
			case 0:
				bitmask = 1 << 9;
				break;
			case 1:
				bitmask = 1 << 3;
				break;
			case 2:
				bitmask = 1 << 5;
				break;
			case 3:
				bitmask = 1 << 7;
				break;
		}
		//check falling edge and increment TIMA
		if((oldClock & bitmask) && !(newClock & bitmask))
		{
			TIMA++;
			//check TIMA overflow, trigger reset, request timer interrupt
			if(!TIMA)
			{
				gb.TIMA_scheduled = 4;
				gb.sysbus[0xFF0F] |= 0x04;
				gb.sysbus[0xFF05] = 0x00;
			}
			else
				gb.sysbus[0xFF05] = TIMA;
		}
	}
}

void gb_exit_invalid_opcode(uint8_t instruction)
{
	printf("Invalid opcode: 0x%02x\n", instruction);
	exit(1);
}

uint8_t getSTATint()
{
	uint8_t STAT = gb.sysbus[STAT_ADDR];

	//check LYC == LY
	if((STAT & 0x40) && (STAT & 0x04))
		return 1;

	//check all mode changes except pixel transfer
	switch(gb.ppu_mode)
	{
		case 0:
			if(STAT & 0x08)
				return 1;
			return 0;
		case 1:
			if(STAT & 0x10)
				return 1;
			return 0;
		case 2:
			if(STAT & 0x20)
				return 1;
			return 0;
		default:
			return 0;
	}
}
