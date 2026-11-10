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
		if(gb.apu.ch1.isActive)
		{
			//advance channel 1

			//update frequency timer
			gb.apu.ch1.frequencyTimer--;
			if(gb.apu.ch1.frequencyTimer == 0)
			{
				gb.apu.ch1.frequencyTimer = gb.apu.ch1.reloadFrequency;
				gb.apu.ch1.dutyIndex = (gb.apu.ch1.dutyIndex + 1) % 8;
			}
		}

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

		if(gb.apu.ch4.isActive)
		{
			//advance channel 4
			if(gb.apu.ch4.clockShift < 14)
			{
				//update frequency timer
				gb.apu.ch4.frequencyTimer--;
				if(gb.apu.ch4.frequencyTimer == 0)
				{
					gb.apu.ch4.frequencyTimer = getCH4ReloadFrequency();
					apu_ch4_lfsr_step();
				}
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
			if(gb.apu.ch1.lengthEnable && gb.apu.ch1.lengthCounter > 0) gb.apu.ch1.lengthCounter--;
			if(gb.apu.ch2.lengthEnable && gb.apu.ch2.lengthCounter > 0) gb.apu.ch2.lengthCounter--;
			if(gb.apu.ch4.lengthEnable && gb.apu.ch4.lengthCounter > 0) gb.apu.ch4.lengthCounter--;

			//if length counter hit zero, stop playing
			if(gb.apu.ch1.lengthCounter == 0) gb.apu.ch1.isActive = 0;
			if(gb.apu.ch2.lengthCounter == 0) gb.apu.ch2.isActive = 0;
			if(gb.apu.ch4.lengthCounter == 0) gb.apu.ch4.isActive = 0;
		}

		//update channel 1 sweep timer
		if(gb.apu.frameSeqStep == 2 || gb.apu.frameSeqStep == 6)
			apu_ch1_sweep_tick();

		//update envelopes (on step 7)
		if(gb.apu.frameSeqStep == 7)
		{
			if(gb.apu.ch1.envelopePeriod > 0)
			{
				//at the end of every period,

				//update channel 1 envelope
				if(gb.apu.ch1.envelopeTimer == 0)
				{
					//reset envelope timer
					gb.apu.ch1.envelopeTimer = gb.apu.ch1.envelopePeriod;

					//update volume based on envelope direction
					if(gb.apu.ch1.envelopeDirection == 1)
					{
						if(gb.apu.ch1.currentVolume < 15) //apply clamping, volume options range only from 0-15
							gb.apu.ch1.currentVolume++;
					}
					else
						if(gb.apu.ch1.currentVolume > 0) //apply clamping, volume options range only from 0-15
							gb.apu.ch1.currentVolume--;
				}
				else
					//advance envelope timer
					gb.apu.ch1.envelopeTimer--;
			}

			if(gb.apu.ch2.envelopePeriod > 0)
			{
				//at the end of every period,

				//update channel 2 envelope
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

			if(gb.apu.ch4.envelopePeriod > 0)
			{
				//at the end of every period,

				//update channel 4 envelope
				if(gb.apu.ch4.envelopeTimer == 0)
				{
					//reset envelope timer
					gb.apu.ch4.envelopeTimer = gb.apu.ch4.envelopePeriod;

					//update volume based on envelope direction
					if(gb.apu.ch4.envelopeDirection == 1)
					{
						if(gb.apu.ch4.currentVolume < 15) //apply clamping, volume options range only from 0-15
							gb.apu.ch4.currentVolume++;
					}
					else
						if(gb.apu.ch4.currentVolume > 0) //apply clamping, volume options range only from 0-15
							gb.apu.ch4.currentVolume--;
				}
				else
					//advance envelope timer
					gb.apu.ch4.envelopeTimer--;
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

uint8_t apu_get_ch1_output()
{
	//Channel is silent if channel is disabled or DAC is disabled
	if(!gb.apu.ch1.dacEnable || !gb.apu.ch1.isActive) return 0;

	//return current output of channel 1
	int currWaveStep = (dutyPatterns[gb.apu.ch1.dutyPattern] >> gb.apu.ch1.dutyIndex) & 0x01; //0 or 1
	return currWaveStep * gb.apu.ch1.currentVolume; //apply envelope
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
	//APU off
	if(gb.apu.masterEnable == 0)
	{
		//Allow writes to lower 6 bits of NRx1 registers
		if(addr == NR11_ADDR || addr == NR21_ADDR/* || addr == NR31_ADDR*/ || addr == NR41_ADDR)
			val &= 0x3F;
		//Allow writes to NR52 master control
		else if(addr != NR52_ADDR)
			return;
		//All other registers are read-only
	}

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

	//CH1
	if(addr == NR10_ADDR)
	{
		//record old pace
		uint8_t oldPace = gb.apu.ch1.sweepPace;

		//update sweep details
		gb.apu.ch1.sweepStep = val & 0x07;
		gb.apu.ch1.sweepDirection = (val >> 3) & 0x01;
		gb.apu.ch1.sweepPace = (val >> 4) & 0x07;

		//disable sweep if new pace is zero
		if(gb.apu.ch1.sweepPace == 0)
			gb.apu.ch1.sweepTimer = 0;
		//otherwise, restart timer if it was previously disabled
		else if(oldPace == 0)
			gb.apu.ch1.sweepTimer = gb.apu.ch1.sweepPace;

		return;
	}

	if(addr == NR11_ADDR)
	{
		//update duty pattern (bits 7-6)
		gb.apu.ch1.dutyPattern = val >> 6;
		//update length counter
		gb.apu.ch1.lengthCounter = 64 - (val & 0x3F);

		return;
	}

	if(addr == NR12_ADDR)
	{
		//set DAC enable based on upper 5 bits
		gb.apu.ch1.dacEnable = (val & 0xF8) != 0;
		//deactivate channel if DAC enable is 0
		if(gb.apu.ch1.dacEnable == 0) gb.apu.ch1.isActive = 0;
		//set initial envelope volume
		gb.apu.ch1.envelopeVolume = val >> 4;
		//set envelope direction (fade in/out)
		gb.apu.ch1.envelopeDirection = (val >> 3) & 0x01;
		//set envelope period
		gb.apu.ch1.envelopePeriod = val & 0x07;
		return;
	}

	if(addr == NR13_ADDR)
	{
		//update lower 8 bits of CH1 period value
		gb.apu.ch1.currFrequency = (gb.apu.ch1.currFrequency & 0x700) | val;
		gb.apu.ch1.reloadFrequency = getReloadFrequency(gb.apu.ch1.currFrequency); //update reload frequency
		return;
	}

	if(addr == NR14_ADDR)
	{
		//update upper 3 bits of period value
		gb.apu.ch1.currFrequency = (gb.apu.ch1.currFrequency & 0xFF) | ((val & 0x07) << 8);
		gb.apu.ch1.reloadFrequency = getReloadFrequency(gb.apu.ch1.currFrequency); //update reload frequency

		//check trigger and enable CH1
		if((val >> 7) == 1)
		{
			//reset frequency timer (preserve lower 2 bits)
			gb.apu.ch1.frequencyTimer = gb.apu.ch1.reloadFrequency | (gb.apu.ch1.frequencyTimer & 0x0003);
			//reset volume
			gb.apu.ch1.currentVolume = gb.apu.ch1.envelopeVolume;
			//reset length timer if expired
			if(gb.apu.ch1.lengthCounter == 0)
				gb.apu.ch1.lengthCounter = 64;
			//reset envelope timer
			gb.apu.ch1.envelopeTimer = gb.apu.ch1.envelopePeriod;

			//trigger channel ONLY if DAC is enabled
			gb.apu.ch1.isActive = gb.apu.ch1.dacEnable;
			apu_ch1_sweep_trigger();
		}

		//update length enable
		gb.apu.ch1.lengthEnable = (val >> 6) & 0x01;
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
		gb.apu.ch2.reloadFrequency = getReloadFrequency(gb.apu.ch2.currFrequency); //update reload frequency
		return;
	}

	if(addr == NR24_ADDR)
	{
		//update upper 3 bits of period value
		gb.apu.ch2.currFrequency = (gb.apu.ch2.currFrequency & 0xFF) | ((val & 0x07) << 8);
		gb.apu.ch2.reloadFrequency = getReloadFrequency(gb.apu.ch2.currFrequency); //update reload frequency

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


	//CH4
	if(addr == NR41_ADDR)
	{
		//update length counter (lower 6 bits)
		gb.apu.ch4.lengthCounter = 64 - (val & 0x3F);
		return;
	}

	if(addr == NR42_ADDR)
	{
		//set DAC enable based on upper 5 bits
		gb.apu.ch4.dacEnable = (val & 0xF8) != 0;
		//deactivate channel if DAC enable is 0
		if(gb.apu.ch4.dacEnable == 0) gb.apu.ch4.isActive = 0;
		//set initial envelope volume
		gb.apu.ch4.envelopeVolume = val >> 4;
		//set envelope direction (fade in/out)
		gb.apu.ch4.envelopeDirection = (val >> 3) & 0x01;
		//set envelope period
		gb.apu.ch4.envelopePeriod = val & 0x07;
		return;
	}

	if(addr == NR43_ADDR)
	{
		gb.apu.ch4.clockShift = (val >> 4) & 0x0F;
		gb.apu.ch4.is7bit = (val >> 3) & 0x01;
		gb.apu.ch4.clockDivider = val & 0x07;
		return;
	}

	if(addr == NR44_ADDR)
	{
		//check trigger
		if((val >> 7) == 1)
		{
			gb.apu.ch4.isActive = gb.apu.ch4.dacEnable; //enable CH4
			if(gb.apu.ch4.lengthCounter == 0) gb.apu.ch4.lengthCounter = 64; //reset length counter
			gb.apu.ch4.envelopeTimer = gb.apu.ch4.envelopePeriod; //reset envelope timer
			gb.apu.ch4.currentVolume = gb.apu.ch4.envelopeVolume; //reset volume
			gb.apu.ch4.lfsr = 0; //reset LFSR
			gb.apu.ch4.frequencyTimer = getCH4ReloadFrequency(); //reset frequency timer
		}

		//update length enable
		gb.apu.ch4.lengthEnable = (val >> 6) & 0x01;
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
				| (gb.apu.ch4.isActive & 0x01) << 3
				//| gb.apu.ch3.isActive & 0x01 << 2
				| (gb.apu.ch2.isActive & 0x01) << 1
				| (gb.apu.ch1.isActive & 0x01)
				);


	//reading from NR10
	if(addr == NR10_ADDR)
		return 0x80
		| (gb.apu.ch1.sweepPace << 4)
		| (gb.apu.ch1.sweepDirection << 3)
		| (gb.apu.ch1.sweepStep);

	//reading from NR11
	if(addr == NR11_ADDR)
		//return duty pattern only (remaining bits are write-only)
		return (gb.apu.ch1.dutyPattern << 6) | 0x3F;

	//reading from NR12
	if(addr == NR12_ADDR)
		//return NR12
		return (gb.apu.ch1.envelopeVolume << 4) | (gb.apu.ch1.envelopeDirection << 3) | gb.apu.ch1.envelopePeriod;

	//reading from NR13
	if(addr == NR13_ADDR)
		//NR13 is write-only
		return 0xFF;

	if(addr == NR14_ADDR)
		//only bit 6 is readable
		return (gb.apu.ch1.lengthEnable << 6) | 0xBF;


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

	//reading from NR24
	if(addr == NR24_ADDR)
		//only bit 6 is readable
		return (gb.apu.ch2.lengthEnable << 6) | 0xBF;

	//reading from NR41
	if(addr == NR41_ADDR)
		//NR41 is write-only
		return 0xFF;

	//reading from NR42
	if(addr == NR42_ADDR)
		//return NR42
		return (gb.apu.ch4.envelopeVolume << 4) | (gb.apu.ch4.envelopeDirection << 3) | gb.apu.ch4.envelopePeriod;

	//reading from NR43
	if(addr == NR43_ADDR)
		//NR43 is write-only
		return 0xFF;

	//reading from NR44
	if(addr == NR44_ADDR)
		//trigger is write-only, length enable is readable
		return 0xBF | (gb.apu.ch4.lengthEnable << 6);

	return 0xFF;
}


void apu_mix_sample()
{
	int16_t left = 0, right = 0;

	//mix channel 1
	if(gb.apu.ch1.dacEnable && gb.apu.ch1.isActive)
	{
		//get current pattern value (0/1)
		int8_t ch1Val = ((dutyPatterns[gb.apu.ch1.dutyPattern]) >> gb.apu.ch1.dutyIndex) & 0x01;
		//apply volume (0..15)
		ch1Val *= gb.apu.ch1.currentVolume;
		//center around 0 (-15..15)
		ch1Val = 15 - (ch1Val * 2);

		//add current ch1 output to global left and right output, apply global volume
		//left, right: 1..8
		if(gb.apu.ch1Left) left += ch1Val * (gb.apu.volLeft + 1);
		if(gb.apu.ch1Right) right += ch1Val * (gb.apu.volRight + 1);
	}

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

	//mix channel 4
	if(gb.apu.ch4.dacEnable && gb.apu.ch4.isActive)
	{
		//get current lfsr value (0/1)
		int8_t ch4Val = gb.apu.ch4.lfsr & 0x01;
		//apply volume (0..15)
		ch4Val *= gb.apu.ch4.currentVolume;
		//center around 0 (-15..15)
		ch4Val = 15 - (ch4Val * 2);

		//add current ch4 output to global left and right output, apply global volume
		//left, right: 1..8
		if(gb.apu.ch4Left) left += ch4Val * (gb.apu.volLeft + 1);
		if(gb.apu.ch4Right) right += ch4Val * (gb.apu.volRight + 1);
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

uint16_t apu_get_ch1_sweep(uint16_t shadow)
{
	//return shadow +- sweepStep
	uint16_t delta = shadow >> gb.apu.ch1.sweepStep;
	return (gb.apu.ch1.sweepDirection == 1) ? shadow - delta : shadow + delta;
}

void apu_ch1_sweep_tick()
{
	//skip if sweep disabled
	if(gb.apu.ch1.sweepPace == 0)
		return;

	//advance sweep timer
	if(gb.apu.ch1.sweepTimer > 0)
		gb.apu.ch1.sweepTimer--;

	if(gb.apu.ch1.sweepTimer != 0)
		return;

	//when timer reaches zero,

	//reset timer
	gb.apu.ch1.sweepTimer = gb.apu.ch1.sweepPace;

	uint16_t nextSweepValue = apu_get_ch1_sweep(gb.apu.ch1.sweepShadow);
	//check overflow, disable CH1 if needed
	if(nextSweepValue > 0x7FF)
	{
		gb.apu.ch1.isActive = 0;
		return;
	}

	//update frequency
	if(gb.apu.ch1.sweepStep != 0)
	{
		gb.apu.ch1.sweepShadow = nextSweepValue;
		gb.apu.ch1.currFrequency = nextSweepValue;
		gb.apu.ch1.reloadFrequency = getReloadFrequency(nextSweepValue);

		//check overflow, disable CH1 if needed
		if(apu_get_ch1_sweep(gb.apu.ch1.sweepShadow) > 0x7FF)
			gb.apu.ch1.isActive = 0;
	}
}

void apu_ch1_sweep_trigger()
{
	//update shadow to current frequency
	gb.apu.ch1.sweepShadow = gb.apu.ch1.currFrequency & 0x7FF;
	//reset sweep timer
	gb.apu.ch1.sweepTimer = gb.apu.ch1.sweepPace;

	//disable CH1 if overflow occurs
	if(gb.apu.ch1.sweepStep != 0 && apu_get_ch1_sweep(gb.apu.ch1.sweepShadow) > 0x7FF)
		gb.apu.ch1.isActive = 0;


}



uint32_t getCH4ReloadFrequency()
{
	//convert NR43 fields to T-cycle value
	if(gb.apu.ch4.clockShift >= 14) return 0; //channel stopped
	if(gb.apu.ch4.clockDivider == 0) return (1 << (gb.apu.ch4.clockShift + 3));
	else return (gb.apu.ch4.clockDivider << (gb.apu.ch4.clockShift + 4));
}

void apu_ch4_lfsr_step()
{
	//take xnor of LFSR bit 0 and 1
	uint16_t *lfsr = &gb.apu.ch4.lfsr;
	uint8_t feedback = !((*lfsr & 1) ^ ((*lfsr >> 1) & 0x01));

	//step lfsr forward
	*lfsr >>= 1;

	//write feedback to bit 14
	if(feedback == 1)
	{
		*lfsr |= 0x4000;
		//also write feedback to bit 6 if LFSR is in 7-bit mode
		if(gb.apu.ch4.is7bit)
			*lfsr |= 0x40;
	}
	else
	{
		*lfsr &= 0xBFFF;
		//also write feedback to bit 6 if LFSR is in 7-bit mode
		if(gb.apu.ch4.is7bit)
			*lfsr &= 0xBF;
	}

}



uint16_t getReloadFrequency(uint16_t currFrequency)
{
	return (2048 - currFrequency) * 4;
}
