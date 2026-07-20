#include <stdint.h>

#ifndef INSTRUCT_H
#define INSTRUCT_H

//load instructions
void ld_r16_imm16(uint8_t r16);
void ld_R16MEM_a(uint8_t r16mem);
void ld_a_R16MEM(uint8_t r16mem);
void ld_IMM16_sp();

//r16 manipulation
void inc_r16(uint8_t r16);
void dec_r16(uint8_t r16);
void add_hl_r16(uint8_t r16);

// bit manipulation
void rlca();
void rrca();
void rla();
void rra();
void cpl();
void scf();
void ccf();

//misc
void daa();
void nop();
void stop();

//helpers
uint16_t *get_r16(uint8_t r16);
uint16_t *get_r16mem(uint8_t r16mem, int *offset);

#endif
