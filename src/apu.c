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
	gb.apu.ch2.frequencyTimer--;
	if(gb.apu.ch2.frequencyTimer == 0)
	{
		gb.apu.ch2.frequencyTimer = gb.apu.ch2.reloadFrequency;
		gb.apu.ch2.dutyIndex = (gb.apu.ch2.dutyIndex + 1) % 8;
	}

	//update frame sequencer
	gb.apu.frameSeqCycles++;
	if(gb.apu.frameSeqCycles == 8192) //one frame sequencer step every 8192 timer ticks
	{
		//step frame sequencer
		gb.apu.frameSeqCycles = 0;
		gb.apu.frameSeqStep = (gb.apu.frameSeqStep + 1) % 8;

		//apply step effects (on step 0,2,4,6)
		if(gb.apu.frameSeqStep % 2 == 0)
		{
			//advance length counter if > 0
			if(gb.apu.ch2.lengthEnable && gb.apu.ch2.lengthCounter > 0)
				gb.apu.ch2.lengthCounter--;

			//if length counter hit zero, stop playing
			if(gb.apu.ch2.lengthCounter == 0)
				gb.apu.ch2.isActive = 0;
		}

		//update envelopes (on step 7)
		if(gb.apu.frameSeqStep == 7)
		{
			if(gb.apu.ch2.envelopePeriod > 0)
			{
				//advance envelope timer
				if(gb.apu.ch2.envelopeTimer > 0)
					gb.apu.ch2.envelopeTimer--;

				//at the end of every period,
				if(gb.apu.ch2.envelopeTimer == 0)
				{
					//reset envelope timer
					gb.apu.ch2.envelopeTimer = gb.apu.ch2.envelopePeriod;

					//update volume based on envelope direction
					if(gb.apu.ch2.envelopeDirection == 1)
					{
						if(gb.apu.ch2.currentVolume < 15) //apply clamping, volume options range only from 0-15
							gb.apu.ch2.currentVolume++;
					}
					else
						if(gb.apu.ch2.currentVolume > 0) //apply clamping, volume options range only from 0-15
							gb.apu.ch2.currentVolume--;
				}
			}
		}
	}

}

uint8_t apu_get_ch2_output()
{
	//Channel is silent if channel is disabled or DAC is disabled
	if(!gb.apu.ch2.dacEnable || !gb.apu.ch2.isActive) return 0;

	//return current output of channel 2
	int currWaveStep = (dutyPatterns[gb.apu.ch2.dutyPattern] >> gb.apu.ch2.dutyIndex) & 0x01; //0 or 1
	return currWaveStep * gb.apu.ch2.currentVolume; //apply envelope
}

void apu_write(uint16_t addr, uint8_t val)
{
	//CH2
	if(addr == NR21_ADDR)
	{
		//update duty pattern (bits 7-6)
		gb.apu.ch2.dutyPattern = val >> 6;
		//update length counter
		gb.apu.ch2.lengthCounter = val & 0x3F;

		return;
	}

	if(addr == NR22_ADDR)
	{
		//set DAC enable based on upper 5 bits
		gb.apu.ch2.dacEnable = (val & 0xF8) != 0;
		//set initial envelope volume
		gb.apu.ch2.envelopeVolume = val >> 4;
		//set envelope direction (fade in/out)
		gb.apu.ch2.envelopeDirection = (val >> 3) & 0x01;
		//set envelope period
		gb.apu.ch2.envelopePeriod = val & 0x07;
		return;
	}

	if(addr == NR23_ADDR)
	{
		//update lower 8 bits of CH2 period value
		gb.apu.ch2.currFrequency = (gb.apu.ch2.currFrequency & 0x700) | val;
		gb.apu.ch2.reloadFrequency = 2048 - gb.apu.ch2.currFrequency; //update reload frequency
		return;
	}

	if(addr == NR24_ADDR)
	{
		//update upper 3 bits of period value
		gb.apu.ch2.currFrequency = (gb.apu.ch2.currFrequency & 0xFF) | ((val & 0x07) << 8);
		gb.apu.ch2.reloadFrequency = 2048 - gb.apu.ch2.currFrequency; //update reload frequency

		//check trigger and enable CH2
		if((val >> 7) == 1)
		{
			//reset frequency timer
			gb.apu.ch2.frequencyTimer = gb.apu.ch2.reloadFrequency;
			//reset duty
			gb.apu.ch2.dutyIndex = 0;
			//reset volume
			gb.apu.ch2.currentVolume = gb.apu.ch2.envelopeVolume;
			//reset length timer if expired
			if(gb.apu.ch2.lengthCounter == 0)
				gb.apu.ch2.lengthCounter = 64;
			//reset envelope timer
			gb.apu.ch2.envelopeTimer = gb.apu.ch2.envelopePeriod;

			//trigger channel ONLY if DAC is enabled
			gb.apu.ch2.isActive = gb.apu.ch2.dacEnable;
		}

		//update length enable
		gb.apu.ch2.lengthEnable = (val >> 6) & 0x01;
	}
}
uint8_t apu_read(uint16_t addr)
{
	//reading from NR21
	if(addr == NR21_ADDR)
		//return duty pattern and length counter
		return (gb.apu.ch2.dutyPattern << 6) | (gb.apu.ch2.lengthCounter & 0x3F);

	//NR22, NR23, NR24 are write-only
	return 0xFF;
}
