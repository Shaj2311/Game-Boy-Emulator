#ifndef APU_H
#define APU_H
#include <stdint.h>

typedef struct
{
	uint16_t currFrequency;		//current frequency of channel
	uint16_t frequencyTimer;	//down counter
	uint16_t reloadFrequency;	//reset to this when timer hits zero

	uint8_t lengthEnable;
	uint8_t lengthCounter;		//max duration of currently playing wave

	uint8_t dutyPattern;		//50%, 25% etc.
	uint8_t dutyIndex;		//current position in duty pattern

	uint8_t dacEnable;		// Enable for DAC (Digital-Analog Converter)
	uint8_t isActive;		//channel enable (depends on DAC)

	uint8_t currentVolume;

	//envelope details
	uint8_t envelopeTimer;
	uint8_t envelopeVolume;
	uint8_t envelopeDirection;
	uint8_t envelopePeriod;

	//sweep details
	uint8_t sweepPace;	//how often a sweep iteration runs
	uint8_t sweepDirection;	//in which direction the frequency changes
	uint8_t sweepStep;	//step taken at every tick of sweep timer
	uint8_t sweepTimer;	//separate timer for sweeps
	uint16_t sweepShadow;	//copy of channel period

} CH1;

typedef struct
{
	uint16_t currFrequency;		//current frequency of channel
	uint16_t frequencyTimer;	//down counter
	uint16_t reloadFrequency;	//reset to this when timer hits zero

	uint8_t lengthEnable;
	uint8_t lengthCounter;		//max duration of currently playing wave

	uint8_t dutyPattern;		//50%, 25% etc.
	uint8_t dutyIndex;		//current position in duty pattern

	uint8_t dacEnable;		// Enable for DAC (Digital-Analog Converter)
	uint8_t isActive;		//channel enable (depends on DAC)

	uint8_t currentVolume;

	//envelope details
	uint8_t envelopeTimer;
	uint8_t envelopeVolume;
	uint8_t envelopeDirection;
	uint8_t envelopePeriod;

} CH2;

typedef struct
{
	uint16_t currFrequency;		//current frequency of channel
	uint16_t frequencyTimer;	//down counter
	uint16_t reloadFrequency;	//reset to this when timer hits zero

	uint8_t lengthEnable;
	uint16_t lengthCounter;		//max duration of currently playing wave

	uint8_t dacEnable;		// Enable for DAC (Digital-Analog Converter)
	uint8_t isActive;		//channel enable (depends on DAC)


	uint8_t outputLevel;		//current volume (mute/100%/50%/25%)

	uint8_t waveIndex;
	uint8_t waveRam[16];
} CH3;

typedef struct
{
	uint16_t lfsr;			//linear feedback shift register

	uint32_t frequencyTimer;	//down counter

	uint8_t clockShift;
	uint8_t clockDivider;
	uint8_t is7bit;

	uint8_t lengthEnable;
	uint8_t lengthCounter;		//max duration of currently playing wave

	uint8_t dacEnable;		// Enable for DAC (Digital-Analog Converter)
	uint8_t isActive;		//channel enable (depends on DAC)

	uint8_t currentVolume;

	//envelope details
	uint8_t envelopeTimer;
	uint8_t envelopeVolume;
	uint8_t envelopeDirection;
	uint8_t envelopePeriod;

} CH4;

typedef struct
{
	//Channels
	CH1 ch1;
	CH2 ch2;
	CH3 ch3;
	CH4 ch4;

	//frame sequencer info
	uint8_t frameSeqStep;
	uint32_t frameSeqCycles;

	//Master settings
	uint8_t masterEnable; //NR52 bit 7, audio on/off

	uint8_t volLeft;
	uint8_t volRight;

	uint8_t ch1Left;
	uint8_t ch1Right;
	uint8_t ch2Left;
	uint8_t ch2Right;
	uint8_t ch3Left;
	uint8_t ch3Right;
	uint8_t ch4Left;
	uint8_t ch4Right;

	//sample clock for mixing
	uint8_t sampleClock;
	int16_t sampleBuffer[8192];
	uint16_t sampleBufferIndex;

	//DC blocker values
	int32_t dcLeft;
	int32_t dcRight;

} APU;

extern uint8_t dutyPatterns[4];

void apu_timer_tick();
uint8_t apu_get_ch2_output(); //get current value of wave pattern

void apu_write(uint16_t addr, uint8_t val);
uint8_t apu_read(uint16_t addr);

void apu_mix_sample();

//ch1 helpers
uint16_t apu_get_ch1_sweep(uint16_t shadow);
void apu_ch1_sweep_tick();
void apu_ch1_sweep_trigger();

uint16_t getWaveReloadFrequency(uint16_t frequency);

//ch4 helpers
void apu_ch4_lfsr_step();
uint32_t getNoiseReloadFrequency();

//used by CH1 and CH2
uint16_t getPulseReloadFrequency(uint16_t currFrequency);


#endif
