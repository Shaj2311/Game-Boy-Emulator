#include "battery.h"
#include "gb.h"
#include <stdio.h>

void batteryLoad(const char *filePath)
{
	//Get save file
	FILE *saveFile = fopen(filePath, "rb");

	//Skip loading if save file does not exist
	if(!saveFile) return;

	printf("Loading save file %s\n", filePath);

	//load save file into cartridge RAM
	fread(gb.cartridgeRAM, 1, gb.cartridgeRamSize, saveFile);

	fclose(saveFile);
}

void batterySave(const char *filePath)
{
	//Skip saving if not applicable
	if(!gb.hasBattery || !gb.cartridgeRAM || !gb.cartridgeRamSize) return;

	//Get save file
	FILE *saveFile = fopen(filePath, "wb");
	if(!saveFile)
	{
		printf("Error: Could not save game to %s\n", filePath);
		return;
	}

	printf("Saving game to %s\n", filePath);

	//write cartridge RAM to save file
	fwrite(gb.cartridgeRAM, 1, gb.cartridgeRamSize, saveFile);

	fclose(saveFile);
}
