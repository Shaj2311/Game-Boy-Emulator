#include "apu.h"
#include "gb.h"
#include "addr.h"
#include <string.h>

uint8_t dutyPatterns[4] =
{
	0b00000001,
	0b10000001,
	0b10000111,
	0b01111110
};

void apu_timer_tick()
{
	if(gb.apu.masterEnable == 1)
	{
		if(gb.apu.ch2.isActive)
		{
			//advance channel 2

			//update frequency timer
			gb.apu.ch2.frequencyTimer--;
			if(gb.apu.ch2.frequencyTimer == 0)
			{
				gb.apu.ch2.frequencyTimer = gb.apu.ch2.reloadFrequency;
				gb.apu.ch2.dutyIndex = (gb.apu.ch2.dutyIndex + 1) % 8;
			}
		}
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
				else
					//advance envelope timer
					gb.apu.ch2.envelopeTimer--;
			}
		}
	}

	//update sample clock, mix sample every 128 timer ticks
	gb.apu.sampleClock++;
	if (gb.apu.sampleClock == 128)
	{
		gb.apu.sampleClock = 0;
		apu_mix_sample();
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
	//APU off -> ignore all writes except to NR52
	if(gb.apu.masterEnable == 0 && addr != NR52_ADDR)
		return;

	//Master
	if(addr == NR50_ADDR)
	{
		gb.apu.volLeft = (val >> 4) & 0x07;
		gb.apu.volRight = val & 0x07;
		return;
	}

	if(addr == NR51_ADDR)
	{
		gb.apu.ch1Right = val & 0x01;
		gb.apu.ch2Right = (val >> 1) & 0x01;
		gb.apu.ch3Right = (val >> 2) & 0x01;
		gb.apu.ch4Right = (val >> 3) & 0x01;

		gb.apu.ch1Left = (val >> 4) & 0x01;
		gb.apu.ch2Left = (val >> 5) & 0x01;
		gb.apu.ch3Left = (val >> 6) & 0x01;
		gb.apu.ch4Left = (val >> 7) & 0x01;

		return;
	}

	if(addr == NR52_ADDR)
	{
		//if setting APU master enable to zero,
		if(val >> 7 == 0)
		{
			//reset all audio registers, preserve frameSeqCycles (DIV-APU)
			uint32_t cycles = gb.apu.frameSeqCycles;
			memset(&gb.apu, 0, sizeof(gb.apu));
			gb.apu.frameSeqCycles = cycles;
		}
		else
			gb.apu.masterEnable = 1;
		return;
	}

	//CH2
	if(addr == NR21_ADDR)
	{
		//update duty pattern (bits 7-6)
		gb.apu.ch2.dutyPattern = val >> 6;
		//update length counter
		gb.apu.ch2.lengthCounter = 64 - (val & 0x3F);

		return;
	}

	if(addr == NR22_ADDR)
	{
		//set DAC enable based on upper 5 bits
		gb.apu.ch2.dacEnable = (val & 0xF8) != 0;
		//deactivate channel if DAC enable is 0
		if(gb.apu.ch2.dacEnable == 0) gb.apu.ch2.isActive = 0;
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
		gb.apu.ch2.reloadFrequency = (2048 - gb.apu.ch2.currFrequency) * 4; //update reload frequency
		return;
	}

	if(addr == NR24_ADDR)
	{
		//update upper 3 bits of period value
		gb.apu.ch2.currFrequency = (gb.apu.ch2.currFrequency & 0xFF) | ((val & 0x07) << 8);
		gb.apu.ch2.reloadFrequency = (2048 - gb.apu.ch2.currFrequency) * 4; //update reload frequency

		//check trigger and enable CH2
		if((val >> 7) == 1)
		{
			//reset frequency timer (preserve lower 2 bits)
			gb.apu.ch2.frequencyTimer = gb.apu.ch2.reloadFrequency | (gb.apu.ch2.frequencyTimer & 0x0003);
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
	//reading from NR50
	if(addr == NR50_ADDR)
		//VIN values always 0
		return ((gb.apu.volLeft & 0x07) << 4) | (gb.apu.volRight & 0x07);

	//reading from NR51
	if(addr == NR51_ADDR)
		return (
			(gb.apu.ch4Left & 0x01) << 7
			| (gb.apu.ch3Left & 0x01) << 6
			| (gb.apu.ch2Left & 0x01) << 5
			| (gb.apu.ch1Left & 0x01) << 4

			| (gb.apu.ch4Right & 0x01) << 3
			| (gb.apu.ch3Right & 0x01) << 2
			| (gb.apu.ch2Right & 0x01) << 1
			| (gb.apu.ch1Right & 0x01)
			);

	//reading from NR52
	if(addr == NR52_ADDR)
		return (
				(gb.apu.masterEnable & 0x01) << 7
				| 0x70
				//| gb.apu.ch4.isActive & 0x01 << 3
				//| gb.apu.ch3.isActive & 0x01 << 2
				| (gb.apu.ch2.isActive & 0x01) << 1
				//| gb.apu.ch2.isActive & 0x01
				);

	//reading from NR21
	if(addr == NR21_ADDR)
		//return duty pattern only (remaining bits are write-only)
		return (gb.apu.ch2.dutyPattern << 6) | 0x3F;

	//reading from NR22
	if(addr == NR22_ADDR)
		//return NR22
		return (gb.apu.ch2.envelopeVolume << 4) | (gb.apu.ch2.envelopeDirection << 3) | gb.apu.ch2.envelopePeriod;

	//reading from NR23
	if(addr == NR23_ADDR)
		//NR23 is write-only
		return 0xFF;

	if(addr == NR24_ADDR)
		//only bit 6 is readable
		return (gb.apu.ch2.lengthEnable << 6) | 0xBF;
}


void apu_mix_sample()
{
	int16_t left = 0, right = 0;

	//mix channel 2
	if(gb.apu.ch2.dacEnable && gb.apu.ch2.isActive)
	{
		//get current pattern value (0/1)
		int8_t ch2Val = ((dutyPatterns[gb.apu.ch2.dutyPattern]) >> gb.apu.ch2.dutyIndex) & 0x01;
		//apply volume (0..15)
		ch2Val *= gb.apu.ch2.currentVolume;
		//center around 0 (-15..15)
		ch2Val = 15 - (ch2Val * 2);

		//add current ch2 output to global left and right output, apply global volume
		//left, right: 1..8
		if(gb.apu.ch2Left) left += ch2Val * (gb.apu.volLeft + 1);
		if(gb.apu.ch2Right) right += ch2Val * (gb.apu.volRight + 1);
	}

	//apply DC blocking (prevent pops)
	int32_t xLeft = (int32_t)left * (32767/480);
	int32_t xRight = (int32_t)right * (32767/480);

	int32_t yLeft = xLeft - gb.apu.dcLeft;
	int32_t yRight = xRight - gb.apu.dcRight;

	gb.apu.dcLeft += yLeft >> 8;
	gb.apu.dcRight += yRight >> 8;

	if(yLeft > 32767) yLeft = 32767;
	if(yRight > 32767) yRight = 32767;
	if(yLeft < -32768) yLeft = -32768;
	if(yRight < -32768) yRight = -32768;

	//mix global left and right outputs to sample buffer (to be outputted)
	gb.apu.sampleBuffer[gb.apu.sampleBufferIndex++] = (int16_t)yLeft;
	gb.apu.sampleBuffer[gb.apu.sampleBufferIndex++] = (int16_t)yRight;
}
