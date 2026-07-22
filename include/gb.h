#ifndef GB_H
#define GB_H
#include <stdint.h>
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

	//Halt flag
	uint8_t halted;

	//Interrupt Master Enable
	uint8_t IME;
} GameBoy;

extern const uint8_t bootROM[256];

extern GameBoy gb;

void gb_boot();
void gb_load_cartridge(const char *cartridge);
void gb_execute(uint8_t instruction);
void gb_exit_invalid_opcode(uint8_t instruction);

uint8_t mmu_read(uint16_t addr);
void mmu_write(uint16_t addr, uint8_t val);

#endif
