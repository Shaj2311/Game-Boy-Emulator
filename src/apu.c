#include "apu.h"
#include "gb.h"
#include "addr.h"

uint8_t dutyPatterns[4] =
{
	0b00000001,
	0b10000001,
	0b10000111,
	0b01111110
};

void apu_timer_tick()
{
	//advance channel 2

	//update frequency timer
	gb.ch2.frequencyTimer--;
	if(gb.ch2.frequencyTimer == 0)
	{
		gb.ch2.frequencyTimer = gb.ch2.reloadFrequency;
		gb.ch2.dutyIndex = (gb.ch2.dutyIndex + 1) % 8;
	}
}

uint8_t apu_get_ch2_output()
{
	return (dutyPatterns[gb.ch2.dutyPattern] >> gb.ch2.dutyIndex) & 0x01;
}

void apu_write(uint16_t addr, uint8_t val)
{
	//CH2
	if(addr == NR21_ADDR)
	{
		//update duty pattern (bits 7-6)
		gb.ch2.dutyPattern = val >> 6;
		//update length counter
		gb.ch2.lengthCounter = val & 0x3F;

		return;
	}

	if(addr == NR22_ADDR)
	{
		//set DAC enable based on upper 5 bits
		gb.ch2.dacEnable = (val & 0xF8) != 0;
		//set initial envelope volume
		gb.ch2.envelopeVolume = val >> 4;
		//set envelope direction (fade in/out)
		gb.ch2.envelopeDirection = (val >> 3) & 0x01;
		//set envelope period
		gb.ch2.envelopePeriod = val & 0x07;
		return;
	}

	if(addr == NR23_ADDR)
	{
		//update lower 8 bits of CH2 period value
		gb.ch2.currFrequency = (gb.ch2.currFrequency & 0x700) | val;
		gb.ch2.reloadFrequency = 2048 - gb.ch2.currFrequency; //update reload frequency
		return;
	}

	if(addr == NR24_ADDR)
	{
		//update upper 3 bits of period value
		gb.ch2.currFrequency = (gb.ch2.currFrequency & 0xFF) | ((val & 0x07) << 8);
		gb.ch2.reloadFrequency = 2048 - gb.ch2.currFrequency; //update reload frequency

		//check trigger and enable CH2
		if((val >> 7) == 1)
		{
			//reset frequency timer
			gb.ch2.frequencyTimer = gb.ch2.reloadFrequency;
			//reset duty
			gb.ch2.dutyIndex = 0;
			//reset volume
			gb.ch2.currentVolume = gb.ch2.envelopeVolume;
			//reset length timer if expired
			if(gb.ch2.lengthCounter == 0)
				gb.ch2.lengthCounter = 64;

			//trigger channel ONLY if DAC is enabled
			gb.ch2.isActive = gb.ch2.dacEnable;
		}
	}
}
uint8_t apu_read(uint16_t addr)
{
	//reading from NR21
	if(addr == NR21_ADDR)
		//return duty pattern and length counter
		return (gb.ch2.dutyPattern << 6) | (gb.ch2.lengthCounter & 0x3F);

	//NR22, NR23, NR24 are write-only
	return 0xFF;
}
