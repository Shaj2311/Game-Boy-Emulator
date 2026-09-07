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
}
uint8_t apu_read(uint16_t addr)
{
	//reading from NR21
	if(addr == NR21_ADDR)
		//return duty pattern and length counter
		return (gb.ch2.dutyPattern << 6) | (gb.ch2.lengthCounter & 0x3F);

	//reading from NR22
	if(addr == NR22_ADDR)
		//NR22 is write-only
		return 0xFF;

	return 0xFF;
}
