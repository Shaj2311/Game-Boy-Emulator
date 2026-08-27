#ifndef MMU_H
#define MMU_H

#include "timers.h"
#include "ppu.h"
#include "addr.h"

uint8_t mmu_read(uint16_t addr)
{
	////DEBUG: Return 0x90 when reading from LY
	//if(addr == LY_ADDR)
	//	return 0x90;

	uint8_t val = gb.sysbus[addr];

	//reading from joypad register
	if(addr == JOYP_ADDR)
		val = gb_compute_joyp();

	//reading from boot ROM
	else if(addr < 0x0100)
	{
		//check boot ROM switch
		if(gb.sysbus[0xFF50] == 0)
		{
			//read from boot ROM
			val = bootROM[addr];
		}
	}

	//reading from switchable ROM bank (0x4000 - 0x7FFF)
	else if(addr <= 0x7FFF)
		return mbcRomRead(addr);

	//reading from external cartridge RAM
	else if(addr >= 0xA000 && addr <= 0xBFFF)
		return mbcRamRead(addr);

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
		mbcRamWrite(addr, val);

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

#endif
