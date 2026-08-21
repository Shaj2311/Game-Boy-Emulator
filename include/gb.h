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
	char *rom;
	long romSize;
	uint8_t currRomBank;

	//Banking details
	uint8_t ramEnable;
	uint8_t ramBankOrRomHigh;
	uint8_t bankingMode;
	uint8_t *cartridgeRAM;
	size_t cartridgeRamSize;
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

void gb_boot();
void gb_init_cartridge_ram();
void gb_load_cartridge(const char *cartridge);
void gb_service_interrupts();
void gb_execute(uint8_t instruction);
void gb_exit_invalid_opcode(uint8_t instruction);
void gb_timer_tick();

uint8_t mmu_read(uint16_t addr);
void mmu_write(uint16_t addr, uint8_t val);

void ppu_set_mode(PPU_Mode mode);
void ppu_set_LY(uint8_t LY);
void ppu_timer_tick();
OAM_Result ppu_oam_search();
void ppu_pixel_transfer();
void ppu_pix_trans_bg(uint8_t X, uint8_t Y, uint8_t SCX, uint8_t SCY, uint8_t LCDC, uint16_t bgTileMapAddr, uint8_t *currBg);
void ppu_pix_trans_win(uint8_t X, uint8_t Y, uint8_t WX, uint8_t WY, uint8_t LCDC, uint16_t winTileMapAddr, uint8_t *currBg);
void ppu_pix_trans_sprites(OAM_Result sprites, uint8_t LCDC, uint8_t *currBg);
uint32_t ppu_lookup_RGBA(uint8_t code, uint8_t paletteReg);

uint8_t getSTATint();
#endif
