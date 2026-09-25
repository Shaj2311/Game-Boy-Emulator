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

} CH2;

typedef struct
{
	//Channels
	CH2 ch2;

	//frame sequencer info
	uint8_t frameSeqStep;
	uint32_t frameSeqCycles;
} APU;

extern uint8_t dutyPatterns[4];

void apu_timer_tick();
uint8_t apu_get_ch2_output(); //get current value of wave pattern

void apu_write(uint16_t addr, uint8_t val);
uint8_t apu_read(uint16_t addr);

#endif
