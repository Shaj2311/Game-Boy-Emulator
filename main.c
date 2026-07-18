#include "gb.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define DBG_CARTRIDGE "dmg_sound.gb"

GameBoy gb;
char *rom;

void gb_boot();
void gb_load_cartridge(const char *cartridge);

uint8_t mmu_read(uint16_t addr);
void mmu_write(uint16_t addr, uint8_t val);

int main()
{
	//boot game boy
	gb_boot();
}

void gb_boot()
{
	//reset CPU registers
	gb.AF = 0;
	gb.BC = 0;
	gb.DE = 0;
	gb.HL = 0;
	gb.SP = 0;
	gb.PC = 0;

	//reset boot ROM mapping control
	gb.sysbus[0xFF50] = 0;

	//load cartridge
	gb_load_cartridge(DBG_CARTRIDGE);
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
	if(itemsRead < romSize)
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
	memcpy(gb.sysbus, rom, 32768);

	puts("Cartridge loaded successfully");
}

uint8_t mmu_read(uint16_t addr)
{
	if(addr <= 0x0100)
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
