#include "apu.h"
#include "gb.h"

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

uint8_t getCh2Output()
{
	return (dutyPatterns[gb.ch2.dutyPattern] >> gb.ch2.dutyIndex) & 0x01;
}
