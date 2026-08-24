#include "gb.h"

void mbcControlWrite(uint16_t addr, uint8_t val)
{
	if(gb.mbcType == MBC_NONE)
		return;

	//writing to RAM enable
	if(addr <= 0x1FFF)
	{
		gb.ramEnable = ((val & 0x0F) == 0x0A);
		return;
	}

	//writing to ROM bank number
	if(addr <= 0x3FFF)
	{
		switch(gb.mbcType)
		{
			case MBC_1:
				val &= 0x1F;
				if(val == 0) val = 1;
				gb.currRomBank = val;
				break;

			case MBC_3:
				val &= 0x7F;
				if(val == 0) val = 1;
				gb.currRomBank = val;
				break;

			case MBC_5:
			default: break;
		}

		return;
	}
}
