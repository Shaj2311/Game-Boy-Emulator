#ifndef GB_H
#define GB_H
#include <stdint.h>

typedef enum
{
	PPU_MODE_HBLANK,
	PPU_MODE_VBLANK,
	PPU_MODE_OAM_SEARCH,
	PPU_MODE_PIX_TRANS
} PPU_Mode;

typedef struct
{
	uint8_t y;
	uint8_t x;
	uint8_t tileIndex;
	uint8_t attr;
} Sprite;

typedef struct
{
	Sprite *sprites[10];
	uint8_t count;
} OAM_Result;

typedef enum
{
	MBC_NONE,
	MBC_1,
	MBC_3,
	MBC_5
} MBC_Type;

typedef enum
{
	// buttons
	INPUT_A,
	INPUT_B,
	INPUT_SELECT,
	INPUT_START,

	// D-pad
	INPUT_RIGHT,
	INPUT_LEFT,
	INPUT_UP,
	INPUT_DOWN,
} JoypadInput;

typedef struct
{
	//CPU registers
	union {uint16_t AF; struct{uint8_t F; uint8_t A;};};
	union {uint16_t BC; struct{uint8_t C; uint8_t B;};};
	union {uint16_t DE; struct{uint8_t E; uint8_t D;};};
	union {uint16_t HL; struct{uint8_t L; uint8_t H;};};
	uint16_t SP;
	uint16_t PC;

	//64KiB system bus
	uint8_t sysbus[65536];

	//system clock
	uint32_t clock;

	//Halt flag
	uint8_t halted;

	//Interrupt Master Enable
	uint8_t IME;

	//IME scheduled (EI)
	uint8_t IME_scheduled;

	//TIMA reload delay
	uint8_t TIMA_scheduled;

	//STAT interrupt checker
	uint8_t STAT_old_int;

	//PPU Details
	uint16_t ppu_cycles;
	PPU_Mode ppu_mode;
	uint32_t frameBuffer[160 * 144];
	uint8_t windowLinesRendered;
	OAM_Result OAM_search_result;

	//ROM details
	char *cartridgeRomPath;
	char *rom;
	long romSize;
	MBC_Type mbcType;
	uint16_t currRomBank;
	uint8_t hasBattery;
	char batterySavePath[512];

	//Banking details
	uint8_t ramEnable;
	uint8_t ramBankReg;
	uint8_t bankingMode;
	uint8_t *cartridgeRAM;
	size_t cartridgeRamSize;

	//Joypad inputs
	uint8_t joypadInputs; //[D-PAD | BUTTONS]
} GameBoy;

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} GB_RGBA;

extern const uint8_t bootROM[256];
extern GameBoy gb;
//extern const GB_RGBA RGBA[4];
extern const uint32_t RGBA[4];

void gb_get_cartridge_path(int argc, char **argv);
void gb_boot();
void gb_init_cartridge_ram();
void gb_load_cartridge(const char *cartridge);
void gb_service_interrupts();
void gb_execute(uint8_t instruction);
void gb_exit_invalid_opcode(uint8_t instruction);

uint8_t gb_compute_joyp();
void gb_press_key(JoypadInput input);
void gb_release_key(JoypadInput input);

void mbcControlWrite(uint16_t addr, uint8_t val);
uint8_t mbcRomRead(uint16_t addr);
void mbcRamWrite(uint16_t addr, uint8_t val);
uint8_t mbcRamRead(uint16_t addr);

uint8_t getSTATint();
#endif
