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
	uint16_t clock;

	//Halt flag
	uint8_t halted;

	//Interrupt Master Enable
	uint8_t IME;

	//IME scheduled (EI)
	uint8_t IME_scheduled;

	//PPU current cycle count
	uint16_t ppu_cycles;

	//PPU mode
	PPU_Mode ppu_mode;
} GameBoy;

extern const uint8_t bootROM[256];
extern GameBoy gb;

void gb_boot();
void gb_load_cartridge(const char *cartridge);
void gb_service_interrupts();
uint8_t gb_execute(uint8_t instruction);
void gb_exit_invalid_opcode(uint8_t instruction);
void gb_timer_tick(uint8_t cycles);

uint8_t mmu_read(uint16_t addr);
void mmu_write(uint16_t addr, uint8_t val);

void ppu_set_mode(PPU_Mode mode);
void ppu_timer_tick(uint16_t cycles);

#endif
