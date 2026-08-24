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
	if(addr <= 0x2FFF)
	{
		switch(gb.mbcType)
		{
			case MBC_1:
				//Mask to lower 5 bits, 0->1
				val &= 0x1F;
				if(val == 0) val = 1;
				gb.currRomBank = val;
				break;

			case MBC_3:
				//Mask to lower 7 bits, 0->1
				val &= 0x7F;
				if(val == 0) val = 1;
				gb.currRomBank = val;
				break;

			case MBC_5:
				//keep 9th bit, update lower 8 bits
				gb.currRomBank = (gb.currRomBank & 0x100) | val;
				break;
			default: break;
		}

		return;
	}

	//writing to ROM bank number (upper region)
	if(addr <= 0x3FFF)
	{
		switch(gb.mbcType)
		{
			case MBC_1:
				//Mask to lower 5 bits, 0->1
				val &= 0x1F;
				if(val == 0) val = 1;
				gb.currRomBank = val;
				break;

			case MBC_3:
				//Mask to lower 7 bits, 0->1
				val &= 0x7F;
				if(val == 0) val = 1;
				gb.currRomBank = val;
				break;

			case MBC_5:
				gb.currRomBank = (gb.currRomBank & 0xFF) | ((val & 0x01) << 8);
				break;

			default: break;
		}
		return;
	}

	//writing to RAM bank or upper ROM
	if(addr <= 0x5FFF)
	{
		switch(gb.mbcType)
		{
			case MBC_1:
				gb.ramBankReg = val & 0x03;
				break;
			//TODO
			case MBC_3:
				gb.ramBankReg = val & 0x0F;
				break;
			case MBC_5:
				gb.ramBankReg = val & 0x0F;
				break;

			default: break;
		}
		return;
	}

	//writing to banking mode select / RTC latch
	if(addr <= 0x7FFF)
	{
		switch(gb.mbcType)
		{
			case MBC_1:
				gb.bankingMode = val & 0x01;
				break;

			case MBC_3:
				//TODO: latch stuff
				break;

			default: break;
		}
	}
}
