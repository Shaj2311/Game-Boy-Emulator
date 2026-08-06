#include "gb.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// GameBoy constants
#define GB_MASTER_CLOCK_FREQ 4194304
#define GB_FPS 59.73
#define GB_CYCLES_PER_FRAME (GB_MASTER_CLOCK_FREQ / GB_FPS)
#define GB_MS_PER_FRAME (1000.0 / GB_FPS)

int main(int argc, char **argv)
{
	//ignore args for now
	(void)argc;
	(void)argv;

	//get performance frequency (to convert ticks to milliseconds)
	uint64_t sdlPerfFreq = SDL_GetPerformanceFrequency();

	//boot game boy
	gb_boot();

	puts("Executing cartridge...");
	//FDE cycle
	while(1)
	{
		//get start ticks
		uint64_t startTicks = SDL_GetPerformanceCounter();

		uint32_t cycles = 0;
		while(cycles < GB_CYCLES_PER_FRAME)
		{
			//check IME
			if(gb.IME_scheduled == 2)
			{
				//enable interrupts
				gb.IME = 1;
				gb.IME_scheduled = 0;
			}
			else if(gb.IME_scheduled == 1)
			{
				//one instruction delay
				gb.IME_scheduled = 2;
			}

			//service interrupts
			gb_service_interrupts();

			//check if halted
			uint8_t currCycles;
			if(gb.halted)
			{
				currCycles = 4;
			}
			else
			{
				//get instruction
				uint8_t instruction = mmu_read(gb.PC++);

				//execute instruction
				currCycles = gb_execute(instruction);
			}

			//advance cycle count
			gb_timer_tick(currCycles);
			ppu_timer_tick(currCycles);

			//advance total cycles executed in this frame
			cycles += currCycles;
		}

		//get end ticks
		uint64_t endTicks = SDL_GetPerformanceCounter();

		//calculate delta
		double msDelta = ((double)(endTicks - startTicks) / sdlPerfFreq) * 1000;

		//apply frame pacing
		if(msDelta < GB_MS_PER_FRAME)
			SDL_Delay((uint32_t)(GB_MS_PER_FRAME - msDelta));
		//apply spin wait for higher precision pacing
		while(((double)(SDL_GetPerformanceCounter() - startTicks) / sdlPerfFreq) * 1000 < GB_MS_PER_FRAME);
	}
}
