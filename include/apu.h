#ifndef APU_H
#define APU_H
#include <stdint.h>

typedef struct
{
	uint16_t frequencyTimer;	//down counter
	uint16_t reloadFrequency;	//reset to this when timer hits zero
	uint8_t dutyPattern;		//50%, 25% etc.
	uint8_t dutyIndex;		//current position in duty pattern
} CH2;

extern uint8_t dutyPatterns[4];

void apu_timer_tick();
uint8_t getCh2Output(); //get current value of wave pattern

void apu_write(uint16_t addr, uint8_t val);
uint8_t apu_read(uint16_t addr);

#endif
