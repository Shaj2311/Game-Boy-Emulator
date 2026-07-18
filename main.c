#include "gb.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define DBG_CARTRIDGE "dmg_sound.gb"

GameBoy gb;
char *rom;

void gb_boot();
void gb_load_cartridge(const char *cartridge);

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
