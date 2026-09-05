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

void apu_timer_tick();

#endif
