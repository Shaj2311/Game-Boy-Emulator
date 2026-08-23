#include "gb.h"

void gb_press_key(JoypadInput input)
{
	//update joypad keys
	gb.joypadInputs &= ~(1 << input);

	//update JOYP
	gb_compute_joyp();

	//TODO: request joypad interrupt
}

uint8_t gb_compute_joyp()
{
	//get JOYP enables
	uint8_t enables = gb.sysbus[JOYP_ADDR] & 0xF0;

	//update lower nibble based on enables
	uint8_t inputState = 0x0F;
	if((enables & 0x10) == 0) //D-pad enabled
		inputState &= gb.joypadInputs >> 4;
	if((enables & 0x20) == 0) //Buttons enabled
		inputState &= gb.joypadInputs & 0x0F;

	//write new JOYP
	gb.sysbus[JOYP_ADDR] = enables | inputState;

	//return new JOYP
	return gb.sysbus[JOYP_ADDR];
}
