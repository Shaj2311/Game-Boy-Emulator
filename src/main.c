#include "gb.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// GameBoy constants
#define GB_MASTER_CLOCK_FREQ 4194304
#define GB_FPS 59.73
#define GB_CYCLES_PER_FRAME (GB_MASTER_CLOCK_FREQ / GB_FPS)
#define GB_MS_PER_FRAME (1000.0 / GB_FPS)

void dbgLogState(FILE *logFile);

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	//Create window
	SDL_Window *window = SDL_CreateWindow("Game Boy", 160 * 5, 144 * 5, 0);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, 0);
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	SDL_Event event;
	int running = 1;

	//ignore args for now
	(void)argc;
	(void)argv;

	//get performance frequency (to convert ticks to milliseconds)
	uint64_t sdlPerfFreq = SDL_GetPerformanceFrequency();

	//boot game boy
	gb_boot();

	puts("Executing cartridge");
	//FDE cycle
	while(running)
	{
		//Poll events
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_EVENT_QUIT:
					running = 0;
					break;
				case SDL_EVENT_KEY_DOWN:
					if(event.key.repeat) break; //skip if pressed in previous iteration
					switch(event.key.scancode)
					{
						case SDL_SCANCODE_ESCAPE:
							running = 0; break;

						case SDL_SCANCODE_S: case SDL_SCANCODE_DOWN:
							gb_press_key(INPUT_DOWN);
							break;
						case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER:
							gb_press_key(INPUT_START);
							break;

						case SDL_SCANCODE_W: case SDL_SCANCODE_UP:
							gb_press_key(INPUT_UP);
							break;
						case SDL_SCANCODE_TAB: case SDL_SCANCODE_KP_PLUS:
							gb_press_key(INPUT_SELECT);
							break;

						case SDL_SCANCODE_K: case SDL_SCANCODE_KP_2:
							gb_press_key(INPUT_B);
							break;
						case SDL_SCANCODE_A: case SDL_SCANCODE_LEFT:
							gb_press_key(INPUT_LEFT);
							break;

						case SDL_SCANCODE_D: case SDL_SCANCODE_RIGHT:
							gb_press_key(INPUT_RIGHT);
							break;
						case SDL_SCANCODE_J: case SDL_SCANCODE_KP_1:
							gb_press_key(INPUT_A);
							break;

						default: break;
					}
					break;
				case SDL_EVENT_KEY_UP:
					switch(event.key.scancode)
					{
						case SDL_SCANCODE_S: case SDL_SCANCODE_DOWN:
							gb_release_key(INPUT_DOWN);
							break;
						case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER:
							gb_release_key(INPUT_START);
							break;

						case SDL_SCANCODE_W: case SDL_SCANCODE_UP:
							gb_release_key(INPUT_UP);
							break;
						case SDL_SCANCODE_TAB: case SDL_SCANCODE_KP_PLUS:
							gb_release_key(INPUT_SELECT);
							break;

						case SDL_SCANCODE_K: case SDL_SCANCODE_KP_2:
							gb_release_key(INPUT_B);
							break;
						case SDL_SCANCODE_A: case SDL_SCANCODE_LEFT:
							gb_release_key(INPUT_LEFT);
							break;

						case SDL_SCANCODE_D: case SDL_SCANCODE_RIGHT:
							gb_release_key(INPUT_RIGHT);
							break;
						case SDL_SCANCODE_J: case SDL_SCANCODE_KP_1:
							gb_release_key(INPUT_A);
							break;

						default: break;
					}
					break;
			}
		}

		//reset rendered window lines counter (required by PPU to draw window)
		gb.windowLinesRendered = 0;

		//get start ticks
		uint64_t startTicks = SDL_GetPerformanceCounter();

		uint32_t frameStartClock = gb.clock;
		while(gb.clock - frameStartClock < GB_CYCLES_PER_FRAME)
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
			if(gb.halted)
			{
				//halt for 1 M-cycle
				for(int i = 0; i < 4; i++)
				{
					gb_timer_tick();
					ppu_timer_tick();
				}

				continue;
			}

			//DEBUG
			//dbgLogState(logFile);

			//get instruction
			uint8_t instruction = mmu_read(gb.PC++);

			//execute instruction
			gb_execute(instruction);
		}

		//render
		SDL_UpdateTexture(texture, 0, gb.frameBuffer, 160 * sizeof(uint32_t));
		SDL_RenderClear(renderer);
		SDL_RenderTexture(renderer, texture, 0, 0);
		SDL_RenderPresent(renderer);

		//apply frame pacing
		uint64_t endTicks = SDL_GetPerformanceCounter();
		double msDelta = ((double)(endTicks - startTicks) / sdlPerfFreq) * 1000;
		if(msDelta < GB_MS_PER_FRAME)
			SDL_Delay((uint32_t)(GB_MS_PER_FRAME - msDelta));

		//apply spin wait for higher precision pacing
		while(((double)(SDL_GetPerformanceCounter() - startTicks) / sdlPerfFreq) * 1000 < GB_MS_PER_FRAME);
	}

	puts("Exiting");
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	free(gb.cartridgeRAM);
	free(gb.rom);
	return 0;
}

void dbgLogState(FILE *logFile)
{
	fprintf(logFile,
			"A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n",
			gb.A, gb.F, gb.B, gb.C, gb.D, gb.E, gb.H, gb.L, gb.SP, gb.PC,
			gb.sysbus[gb.PC],
			gb.sysbus[gb.PC + 1],
			gb.sysbus[gb.PC + 2],
			gb.sysbus[gb.PC + 3]);
}
