#include "apu.h"
#include "gb.h"

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
