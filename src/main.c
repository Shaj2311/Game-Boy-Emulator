#include "gb.h"
#include "timers.h"
#include "mmu.h"
#include "battery.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// GameBoy constants
#define GB_MASTER_CLOCK_FREQ 4194304
#define GB_FPS 59.73
#define GB_CYCLES_PER_FRAME (GB_MASTER_CLOCK_FREQ / GB_FPS)
#define GB_MS_PER_FRAME (1000.0 / GB_FPS)
#define GB_AUDIO_FREQUENCY 32768

int main(int argc, char **argv)
{
	//get path to cartridge
	gb_get_cartridge_path(argc, argv);

	//Initialize SDL3
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD);

	//Create window
	SDL_Window *window = SDL_CreateWindow("Game Boy", 160 * 5, 144 * 5, 0);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, 0);
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	SDL_Event event;
	int running = 1;

	//Initialize audio
	SDL_AudioSpec spec;
	spec.format = SDL_AUDIO_S16;
	spec.channels = 2;
	spec.freq = GB_AUDIO_FREQUENCY;
	SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
	SDL_AudioStream *audioStream = SDL_CreateAudioStream(&spec, &spec);
	SDL_BindAudioStream(audioDevice, audioStream);

	//Initialize gamepad
	SDL_Gamepad *gamepad = 0;
	SDL_JoystickID gamepadID = 0;

	int padCount = 0;
	SDL_JoystickID *pads = SDL_GetGamepads(&padCount);
	if(pads)
	{
		if(padCount > 0)
		{
			gamepad = SDL_OpenGamepad(pads[0]);
			if(gamepad) gamepadID = SDL_GetGamepadID(gamepad);
		}
		SDL_free(pads);
	}


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
							gb_press_button(INPUT_DOWN);
							break;
						case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER:
							gb_press_button(INPUT_START);
							break;

						case SDL_SCANCODE_W: case SDL_SCANCODE_UP:
							gb_press_button(INPUT_UP);
							break;
						case SDL_SCANCODE_TAB: case SDL_SCANCODE_KP_PLUS:
							gb_press_button(INPUT_SELECT);
							break;

						case SDL_SCANCODE_K: case SDL_SCANCODE_KP_2:
							gb_press_button(INPUT_A);
							break;
						case SDL_SCANCODE_A: case SDL_SCANCODE_LEFT:
							gb_press_button(INPUT_LEFT);
							break;

						case SDL_SCANCODE_D: case SDL_SCANCODE_RIGHT:
							gb_press_button(INPUT_RIGHT);
							break;
						case SDL_SCANCODE_J: case SDL_SCANCODE_KP_1:
							gb_press_button(INPUT_B);
							break;

						default: break;
					}
					break;
				case SDL_EVENT_KEY_UP:
					switch(event.key.scancode)
					{
						case SDL_SCANCODE_S: case SDL_SCANCODE_DOWN:
							gb_release_button(INPUT_DOWN);
							break;
						case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER:
							gb_release_button(INPUT_START);
							break;

						case SDL_SCANCODE_W: case SDL_SCANCODE_UP:
							gb_release_button(INPUT_UP);
							break;
						case SDL_SCANCODE_TAB: case SDL_SCANCODE_KP_PLUS:
							gb_release_button(INPUT_SELECT);
							break;

						case SDL_SCANCODE_K: case SDL_SCANCODE_KP_2:
							gb_release_button(INPUT_A);
							break;
						case SDL_SCANCODE_A: case SDL_SCANCODE_LEFT:
							gb_release_button(INPUT_LEFT);
							break;

						case SDL_SCANCODE_D: case SDL_SCANCODE_RIGHT:
							gb_release_button(INPUT_RIGHT);
							break;
						case SDL_SCANCODE_J: case SDL_SCANCODE_KP_1:
							gb_release_button(INPUT_B);
							break;

						default: break;
					}
					break;

				case SDL_EVENT_GAMEPAD_ADDED: //gamepad connected after event polling started
					if(gamepad) break;
					gamepad = SDL_OpenGamepad(event.gdevice.which);
					if(gamepad) gamepadID = SDL_GetGamepadID(gamepad);
					break;

				case SDL_EVENT_GAMEPAD_REMOVED: //gamepad disconnected after event polling started
					if(event.gdevice.which != gamepadID) break;
					SDL_CloseGamepad(gamepad);
					gamepad = 0;
					gamepadID = 0;
					break;

				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
					if(event.gdevice.which != gamepadID) break;
					switch(event.gbutton.button)
					{
						case SDL_GAMEPAD_BUTTON_START:
							gb_press_button(INPUT_START);
							break;
						case SDL_GAMEPAD_BUTTON_BACK:
							gb_press_button(INPUT_SELECT);
							break;

						case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
							gb_press_button(INPUT_RIGHT);
							break;
						case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
							gb_press_button(INPUT_DOWN);
							break;
						case SDL_GAMEPAD_BUTTON_DPAD_UP:
							gb_press_button(INPUT_UP);
							break;
						case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
							gb_press_button(INPUT_LEFT);
							break;

						case SDL_GAMEPAD_BUTTON_EAST:
						case SDL_GAMEPAD_BUTTON_NORTH:
							gb_press_button(INPUT_A);
							break;
						case SDL_GAMEPAD_BUTTON_SOUTH:
						case SDL_GAMEPAD_BUTTON_WEST:
							gb_press_button(INPUT_B);
							break;
					}
					break;

				case SDL_EVENT_GAMEPAD_BUTTON_UP:
					if(event.gdevice.which != gamepadID) break;
					switch(event.gbutton.button)
					{
						case SDL_GAMEPAD_BUTTON_START:
							gb_release_button(INPUT_START);
							break;
						case SDL_GAMEPAD_BUTTON_BACK:
							gb_release_button(INPUT_SELECT);
							break;

						case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
							gb_release_button(INPUT_RIGHT);
							break;
						case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
							gb_release_button(INPUT_DOWN);
							break;
						case SDL_GAMEPAD_BUTTON_DPAD_UP:
							gb_release_button(INPUT_UP);
							break;
						case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
							gb_release_button(INPUT_LEFT);
							break;

						case SDL_GAMEPAD_BUTTON_EAST:
							gb_release_button(INPUT_A);
							break;
						case SDL_GAMEPAD_BUTTON_SOUTH:
							gb_release_button(INPUT_B);
							break;
					}
					break;
			}
		}

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
					apu_timer_tick();
				}

				continue;
			}

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

		//output audio
		if(gb.apu.sampleBufferIndex > 0)
		{
			//output audio stream
			SDL_PutAudioStreamData(audioStream, gb.apu.sampleBuffer, gb.apu.sampleBufferIndex * sizeof(int16_t));
			//flush samples
			gb.apu.sampleBufferIndex = 0;
		}

		//apply frame pacing
		uint64_t endTicks = SDL_GetPerformanceCounter();
		double msDelta = ((double)(endTicks - startTicks) / sdlPerfFreq) * 1000;
		if(msDelta < GB_MS_PER_FRAME)
			SDL_Delay((uint32_t)(GB_MS_PER_FRAME - msDelta));

		//apply spin wait for higher precision pacing
		while(((double)(SDL_GetPerformanceCounter() - startTicks) / sdlPerfFreq) * 1000 < GB_MS_PER_FRAME);
	}

	batterySave(gb.batterySavePath);
	puts("Exiting");
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyAudioStream(audioStream);
	SDL_CloseAudioDevice(audioDevice);
	if(gamepad) SDL_CloseGamepad(gamepad);
	free(gb.cartridgeRAM);
	free(gb.rom);
	return 0;
}
