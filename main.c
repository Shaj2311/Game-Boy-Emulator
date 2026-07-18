#include "gb.h"
#include <string.h>
GameBoy gb;
void gb_boot();
void gb_load_rom();
int main()
{
	//reset hardware
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

	//reset I/O registers
	memset(gb.sysbus + 0xFF00, 0, 0xFF7F - 0xFF00 + 1);
}

void gb_load_rom()
{

}
