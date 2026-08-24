#include "gb.h"
#include "instructions.h"
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

uint32_t ppu_lookup_RGBA(uint8_t colorIndex, uint8_t paletteReg)
{
	uint8_t shadeIndex = (paletteReg >> (colorIndex * 2)) & 0x03;
	return RGBA[shadeIndex];
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
	if(gb.rom[0x0147] == 0x00)
		gb.mbcType = MBC_NONE;
	else if(gb.rom[0x0147] <= 0x03)
		gb.mbcType = MBC_1;
	else if(gb.rom[0x0147] <= 0x13)
		gb.mbcType = MBC_3;
	else if(gb.rom[0x0147] <= 0x1E)
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

uint8_t mmu_read(uint16_t addr)
{
	////DEBUG: Return 0x90 when reading from LY
	//if(addr == LY_ADDR)
	//	return 0x90;

	uint8_t val = gb.sysbus[addr];
	//reading from boot ROM
	if(addr < 0x0100)
	{
		//check boot ROM switch
		if(gb.sysbus[0xFF50] == 0)
		{
			//read from boot ROM
			val = bootROM[addr];
		}
	}

	//reading from switchable ROM bank (0x4000 - 0x7FFF)
	else if(addr >= 0x4000 && addr <= 0x7FFF)
	{
		uint8_t bank = gb.currRomBank;

		//default to bank 1
		if(bank == 0)
			bank = 1;

		bank |= (gb.ramBankReg << 5);

		uint32_t offset = (addr - 0x4000) + ((uint32_t)bank * 0x4000);

		//prevent out-of-bounds reads
		offset %= gb.romSize;

		val = gb.rom[offset];
	}

	//reading from external cartridge RAM
	else if(addr >= 0xA000 && addr <= 0xBFFF)
	{
		if(gb.ramEnable && gb.cartridgeRAM != 0)
		{
			uint8_t ramBank = gb.bankingMode ? gb.ramBankReg : 0;
			uint32_t offset = (addr - 0xA000) + (ramBank * 0x2000);

			if(offset < gb.cartridgeRamSize)
				val = gb.cartridgeRAM[offset];
		}
		else
			val = 0xFF;
	}

	//reading from joypad register
	else if(addr == JOYP_ADDR)
		val = gb_compute_joyp();

	//tick timers 1 M-cycle
	for(int i = 0; i < 4; i++)
	{
		gb_timer_tick();
		ppu_timer_tick();
	}

	return val;
}

void mmu_write(uint16_t addr, uint8_t val)
{
	//writing to boot ROM switch
	if(addr == 0xFF50)
	{
		if(!gb.sysbus[0xFF50])
			gb.sysbus[addr] = val;
	}

	//writing to DIV register
	else if(addr == 0xFF04)
	{
		//compute old AND gate output
		uint8_t oldTAC = gb.sysbus[0xFF07];
		uint16_t oldClockBit;
		switch(oldTAC & 3)
		{
			case 0b00: oldClockBit = 512; break;
			case 0b01: oldClockBit = 8; break;
			case 0b10: oldClockBit = 32; break;
			case 0b11: oldClockBit = 128; break;
		}
		uint8_t oldOutput = (oldTAC & 0x04) && (gb.clock & oldClockBit);

		//reset system clock
		gb.clock = 0;
		gb.sysbus[addr] = 0;

		//compute new AND gate output
		uint8_t newTAC = gb.sysbus[0xFF07];
		uint16_t newClockBit;
		switch(newTAC & 3)
		{
			case 0b00: newClockBit = 512; break;
			case 0b01: newClockBit = 8; break;
			case 0b10: newClockBit = 32; break;
			case 0b11: newClockBit = 128; break;
		}
		uint8_t newOutput = (newTAC & 0x04) && (gb.clock & newClockBit);

		//check falling edge and increment TIMA
		if(oldOutput == 1 && newOutput == 0)
		{
			uint8_t TIMA = gb.sysbus[0xFF05];
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

	//writing to LY register
	else if(addr == LY_ADDR)
		//reset register
		gb.sysbus[addr] = 0;

	//writing to STAT register
	else if(addr == STAT_ADDR)
	{
		//get old STAT value
		uint8_t old = gb.sysbus[STAT_ADDR];
		//join with new STAT value
		uint8_t new = (old & 0x07) | (val & 0x78) | 0x80;
		//write new value
		gb.sysbus[addr] = new;

		//request STAT interrupt if needed
		uint8_t STAT_curr_int = getSTATint();
		if(gb.STAT_old_int == 0 && STAT_curr_int == 1)
		{
			gb.sysbus[0xFF0F] |= 0x02;
		}
		gb.STAT_old_int = STAT_curr_int;
	}

	//writing to LYC (that'll change the LYC==LY condition)
	else if(addr == LYC_ADDR)
	{
		gb.sysbus[LYC_ADDR] = val;
		ppu_set_LY(gb.sysbus[LY_ADDR]);
	}

	//writing to TIMA
	else if(addr == 0xFF05)
	{
		//override scheduled TIMA reset
		if(gb.TIMA_scheduled)
			gb.TIMA_scheduled = 0;

		gb.sysbus[addr] = val;

	}

	//writing to TAC
	else if(addr == 0xFF07)
	{
		//compute old AND gate output
		uint8_t oldTAC = gb.sysbus[0xFF07];
		uint16_t oldClockBit;
		switch(oldTAC & 3)
		{
			case 0b00: oldClockBit = 512; break;
			case 0b01: oldClockBit = 8; break;
			case 0b10: oldClockBit = 32; break;
			case 0b11: oldClockBit = 128; break;
		}
		uint8_t oldOutput = (oldTAC & 0x04) && (gb.clock & oldClockBit);

		//write to TAC
		gb.sysbus[addr] = val;

		//compute new AND gate output
		uint8_t newTAC = val;
		uint16_t newClockBit;
		switch(newTAC & 3)
		{
			case 0b00: newClockBit = 512; break;
			case 0b01: newClockBit = 8; break;
			case 0b10: newClockBit = 32; break;
			case 0b11: newClockBit = 128; break;
		}
		uint8_t newOutput = (newTAC & 0x04) && (gb.clock & newClockBit);

		//check falling edge and increment TIMA
		if(oldOutput == 1 && newOutput == 0)
		{
			uint8_t TIMA = gb.sysbus[0xFF05];
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

	//writing to joypad input register
	else if(addr == JOYP_ADDR)
	{
		//get old JOYP
		uint8_t oldJOYP = gb.sysbus[addr];

		//update bits 4 and 5
		oldJOYP &= 0xCF;
		oldJOYP |= (val & 0x30);

		//write to JOYP
		gb.sysbus[addr] = oldJOYP;
	}

	//writing to OAM DMA
	else if(addr == 0xFF46)
	{
		//copy to OAM
		uint16_t srcAddr = val << 8;
		for(int i = 0; i < 160; i++)
		{
			gb.sysbus[0xFE00 + i] = mmu_read(srcAddr + i);
		}
		for(int i = 0; i < 4; i++)
		{
			gb_timer_tick();
			ppu_timer_tick();
		}
	}

	//writing to range 0x0000 to 0x7FFF (ROM region)
	else if(addr <= 0x7FFF)
		mbcControlWrite(addr, val);

	//writing to WRAM (write to echo RAM as well)
	else if(addr >= 0xC000 && addr <= 0xDDFF)
	{
		//write to WRAM
		gb.sysbus[addr] = val;
		//also write to echo RAM
		gb.sysbus[addr + 0x2000] = val;
	}

	//writing to echo RAM (write to WRAM as well)
	else if(addr >= 0xE000 && addr <= 0xFDFF)
	{
		//write to echo RAM
		gb.sysbus[addr] = val;
		//also write to WRAM
		gb.sysbus[addr - 0x2000] = val;
	}

	//writing to cartridge RAM
	else if(addr >= 0xA000 && addr <= 0xBFFF)
	{
		if(gb.ramEnable && gb.cartridgeRAM != 0)
		{
			uint8_t ramBank = gb.bankingMode ? gb.ramBankReg : 0;
			gb.cartridgeRAM[(addr - 0xA000) + ((uint32_t)ramBank * 0x2000)] = val;
		}
	}

	//writing normally
	else
		gb.sysbus[addr] = val;

	//tick timers 1 M-cycle
	for(int i = 0; i < 4; i++)
	{
		gb_timer_tick();
		ppu_timer_tick();
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

void ppu_set_mode(PPU_Mode mode)
{
	uint8_t oldMode = gb.ppu_mode;
	gb.ppu_mode = mode;
	gb.sysbus[STAT_ADDR] = (gb.sysbus[STAT_ADDR] & 0xFC) | mode;

	//request STAT interrupt on mode transition (except pixel transfer mode)
	uint8_t STAT_curr_int = getSTATint();
	if(gb.STAT_old_int == 0 && STAT_curr_int == 1)
	{
		gb.sysbus[0xFF0F] |= 0x02;
	}
	gb.STAT_old_int = STAT_curr_int;

	//execute current PPU mode
	if(gb.ppu_mode != oldMode)
	{
		if(gb.ppu_mode == PPU_MODE_OAM_SEARCH)
			gb.OAM_search_result = ppu_oam_search();
		else if(gb.ppu_mode == PPU_MODE_PIX_TRANS)
			ppu_pixel_transfer();
	}
}

void ppu_set_LY(uint8_t LY)
{
	//write new scanline to LY
	gb.sysbus[LY_ADDR] = LY;

	//update LYC==LY
	if(gb.sysbus[LYC_ADDR] == LY)
	{
		gb.sysbus[STAT_ADDR] |= 0x04;
		//request STAT interrupt
		uint8_t STAT_curr_int = getSTATint();
		if(gb.STAT_old_int == 0 && STAT_curr_int == 1)
		{
			gb.sysbus[0xFF0F] |= 0x02;
		}
		gb.STAT_old_int = STAT_curr_int;
	}
	else
		gb.sysbus[STAT_ADDR] &= ~(0X04);
}

void ppu_timer_tick()
{
	//get LCD control
	uint8_t LCDC = gb.sysbus[0xFF40];
	//check LDC and PPU enable
	if(!(LCDC >> 7))
	{
		//reset cycles
		gb.ppu_cycles = 0;

		//reset mode
		ppu_set_mode(PPU_MODE_HBLANK);

		//reset to top scanline
		ppu_set_LY(0);

		return;
	}

	gb.ppu_cycles++; //tick by 1 cycle
	uint8_t LY = gb.sysbus[LY_ADDR];
	if(LY < 144)
	{
		if(gb.ppu_cycles <= 79)
		{
			//OAM search
			ppu_set_mode(PPU_MODE_OAM_SEARCH);
		}
		else if(gb.ppu_cycles <= 251)
		{
			//Pixel transfer
			ppu_set_mode(PPU_MODE_PIX_TRANS);
		}
		else if(gb.ppu_cycles <= 455)
		{
			//H-blank
			ppu_set_mode(PPU_MODE_HBLANK);
		}
		else
		{
			//End of line reached

			//move to next line
			gb.ppu_cycles -= 456;
			ppu_set_LY(++LY);

			//check vblank
			if(LY == 144)
			{
				//Vblank
				ppu_set_mode(PPU_MODE_VBLANK);
				//request vblank interrupt
				gb.sysbus[0xFF0F] |= 0x01;
			}
			else
			{
				//OAM Search
				ppu_set_mode(PPU_MODE_OAM_SEARCH);
			}
		}
	}
	else
	{
		//vblank
		ppu_set_mode(PPU_MODE_VBLANK);

		//visible scanlines complete
		if(gb.ppu_cycles >= 456)
		{
			gb.ppu_cycles -= 456;

			//if all scanlines complete (including hidden), reset to line 0
			if(LY == 153)
			{
				//reset ppu mode
				ppu_set_mode(PPU_MODE_OAM_SEARCH);

				//reset rendered window lines count
				gb.windowLinesRendered = 0;

				//reset to line 0
				ppu_set_LY(0);
			}
			else
			{
				//move to next line
				ppu_set_LY(++LY);
			}
		}
	}
}

OAM_Result ppu_oam_search()
{
	OAM_Result result;
	result.count = 0;

	uint8_t LCDC = gb.sysbus[LCDC_ADDR];

	//check OBJ enable
	if(!(LCDC & 0x02))
		return result;

	uint8_t spriteHeight = LCDC & 0x04 ? 16 : 8;

	//get LY
	uint8_t LY = gb.sysbus[LY_ADDR];

	//search OAM
	uint16_t addr = 0xFE00;
	while(result.count < 10 && addr <= 0xFE9F)
	{
		//get sprite
		Sprite *ptr = (Sprite *)(gb.sysbus + addr);
		//compare Y with LY
		if(LY + 16 >= ptr->y && LY + 16 < ptr->y + spriteHeight)
			result.sprites[result.count++] = ptr;

		addr += sizeof(Sprite);
	}

	return result;
}

void ppu_pixel_transfer()
{
	//get LCD control
	uint8_t LCDC = gb.sysbus[LCDC_ADDR];

	//get current scanline, set up X and Y
	uint8_t Y = gb.sysbus[LY_ADDR];
	uint8_t X = 0;

	//get OAM search result
	OAM_Result oamSearchResult = gb.OAM_search_result;

	//check LCD & PPU enable
	if(!(LCDC & 0x80))
	{
		//reset screen to white
		uint32_t whiteRGBA = ppu_lookup_RGBA(0, gb.sysbus[BGP_ADDR]);
		for(int x = 0; x < 160; x++)
			gb.frameBuffer[Y * 160 + x] = whiteRGBA;
		return;
	}

	//set up scanline information buffers
	uint8_t currBg[160];

	//get scroll position (SCY, SCX)
	uint8_t SCY = gb.sysbus[SCY_ADDR];
	uint8_t SCX = gb.sysbus[SCX_ADDR];

	//get window position (WY, WX)
	uint8_t WY = gb.sysbus[WY_ADDR];
	uint8_t WX = gb.sysbus[WX_ADDR];

	//get background tilemap starting address
	uint16_t bgTileMapAddr = (LCDC >> 3) & 0x01 ? 0x9C00 : 0x9800;

	//get window tilemap starting address
	uint16_t winTileMapAddr = (LCDC >> 6) & 0x01 ? 0x9C00 : 0x9800;

	//check BG & Window enable
	if((LCDC & 0x01))
	{
		//render background
		ppu_pix_trans_bg(X, Y, SCX, SCY, LCDC, bgTileMapAddr, currBg);

		//render window
		ppu_pix_trans_win(X, Y, WX, WY, LCDC, winTileMapAddr, currBg);
	}
	else
	{
		//don't draw background
		memset(currBg, 0, 160);
	}

	//draw bg/window colors to framebuffer
	uint8_t BGP = gb.sysbus[BGP_ADDR];
	for(int x = 0; x < 160; x++)
	{
		gb.frameBuffer[Y * 160 + x] = ppu_lookup_RGBA(currBg[x], BGP);
	}

	//render sprites
	ppu_pix_trans_sprites(oamSearchResult, LCDC, currBg);
}

void ppu_pix_trans_bg(uint8_t X, uint8_t Y, uint8_t SCX, uint8_t SCY, uint8_t LCDC, uint16_t bgTileMapAddr, uint8_t *currBg)
{
	//render background
	while(X < 160)
	{
		//convert screen coordinates to canvas coordinates
		uint8_t canvasX = X + SCX;
		uint8_t canvasY = Y + SCY;

		//get tile index
		uint16_t tilemapEntryAddr =
			bgTileMapAddr
			+ ((canvasY / 8) * 32)
			+ (canvasX / 8);

		uint8_t tileIndex = gb.sysbus[tilemapEntryAddr];

		//get 2bpp data
		uint8_t currTileBit = canvasY % 8;
		uint16_t tileDataAddr;

		if(LCDC & 0x10) //address depends on signed vs unsigned addressing
			tileDataAddr = 0x8000 + (tileIndex * 16) + (currTileBit * 2);
		else
			tileDataAddr = 0x9000 + ((int8_t)tileIndex * 16) + (currTileBit * 2);

		uint8_t lowByte = gb.sysbus[tileDataAddr];
		uint8_t highByte = gb.sysbus[tileDataAddr + 1];

		//extract and store 2bpp data
		uint8_t bitPosition = 7 - (canvasX % 8);
		uint8_t highBit = (highByte >> bitPosition) & 0x01;
		uint8_t lowBit = (lowByte >> bitPosition) & 0x01;
		uint8_t colorIndex = (highBit << 1) | lowBit;
		currBg[X] = colorIndex;

		//advance through scanline
		X++;
	}
}

void ppu_pix_trans_win(uint8_t X, uint8_t Y, uint8_t WX, uint8_t WY, uint8_t LCDC, uint16_t winTileMapAddr, uint8_t *currBg)
{
	//check window enable
	if(!(LCDC & 0x20))
		return;

	//check position bounds
	if(Y < WY || WX >= (160 + 6))
		return;

	int windowXoffset = (int)WX - 7;
	uint8_t lineRendered = 0;

	while(X < 160)
	{
		if((int)X < windowXoffset)
		{
			X++;
			continue;
		}

		lineRendered = 1;

		//convert screen coordinates to canvas coordinates
		uint8_t canvasX = (int)X - windowXoffset;
		uint8_t canvasY = gb.windowLinesRendered;

		//get tile index
		uint16_t tilemapEntryAddr =
			winTileMapAddr
			+ ((canvasY / 8) * 32)
			+ (canvasX / 8);

		uint8_t tileIndex = gb.sysbus[tilemapEntryAddr];

		//get 2bpp data
		uint8_t currTileBit = canvasY % 8;
		uint16_t tileDataAddr;

		if(LCDC & 0x10) //address depends on signed vs unsigned addressing
			tileDataAddr = 0x8000 + (tileIndex * 16) + (currTileBit * 2);
		else
			tileDataAddr = 0x9000 + ((int8_t)tileIndex * 16) + (currTileBit * 2);

		uint8_t lowByte = gb.sysbus[tileDataAddr];
		uint8_t highByte = gb.sysbus[tileDataAddr + 1];

		//extract and store 2bpp data
		uint8_t bitPosition = 7 - (canvasX % 8);
		uint8_t highBit = (highByte >> bitPosition) & 0x01;
		uint8_t lowBit = (lowByte >> bitPosition) & 0x01;
		uint8_t colorIndex = (highBit << 1) | lowBit;
		currBg[X] = colorIndex;

		//advance through scanline
		X++;
	}

	if(lineRendered)
		gb.windowLinesRendered++;
}

void ppu_pix_trans_sprites(OAM_Result OAM_sprites, uint8_t LCDC, uint8_t *currBg)
{
	//check object enable
	if(!(LCDC & 0x02))
		return;

	Sprite **sprites = OAM_sprites.sprites;
	uint8_t LY = gb.sysbus[LY_ADDR];

	//for each screen pixel on current scanline,
	for(int i = 0; i < 160; i++)
	{
		//search for best sprite
		uint16_t bestX = -1;
		uint8_t bestColorIndex = 0;
		Sprite *bestSprite = 0;
		for(int j = 0; j < OAM_sprites.count; j++)
		{
			//check if current pixel falls within sprite's horizontal bounds
			int16_t spriteX = sprites[j]->x - 8;
			if(!(spriteX <= i && i <= spriteX + 7))
				continue;

			//get tile index, reset bit 0 in case of 8x16 mode
			uint8_t tileIndex = sprites[j]->tileIndex;
			uint8_t spriteHeight = LCDC & 0x04 ? 16 : 8;
			if(spriteHeight == 16)
				tileIndex &= 0xFE;

			//get row inside tile that current pixel is in
			uint8_t tileY = LY - (sprites[j]->y - 16);

			//check vertical flipping (Y-flip)
			if(sprites[j]->attr & 0x40)
				tileY = (spriteHeight - 1) - tileY;

			//get 2bpp
			uint16_t tileDataAddr = 0x8000 + (tileIndex * 16) + (tileY * 2);
			uint8_t lowByte = gb.sysbus[tileDataAddr];
			uint8_t highByte = gb.sysbus[tileDataAddr + 1];

			//check horizontal flip (X-flip)
			uint8_t tileX = i - spriteX;
			uint8_t bitPosition = (sprites[j]->attr & 0x20) ? tileX : (7 - tileX);

			//extract 2bpp data
			uint8_t highBit = (highByte >> bitPosition) & 0x01;
			uint8_t lowBit = (lowByte >> bitPosition) & 0x01;
			uint8_t colorIndex = (highBit << 1) | lowBit;

			//check transparency of pixel
			if(!colorIndex)
				continue;

			//compare with best X
			if(sprites[j]->x < bestX)
			{
				bestX = sprites[j]->x;
				bestSprite = sprites[j];
				bestColorIndex = colorIndex;
			}
		}

		//if no sprites were overlapping on this pixel, skip
		if(!bestSprite)
			continue;

		//check priority
		uint8_t priority = bestSprite->attr >> 7;
		uint8_t currBgColor = currBg[i];
		//if background is enabled and current pixel's bg color has priority over sprite, skip drawing sprite
		if((LCDC & 0x01) && priority && (currBgColor != 0))
			continue;

		//write pixel to framebuffer
		uint8_t palette = bestSprite->attr & 0x10 ? gb.sysbus[OBP1_ADDR] : gb.sysbus[OBP0_ADDR];
		gb.frameBuffer[LY * 160 + i] = ppu_lookup_RGBA(bestColorIndex, palette);
	}
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
