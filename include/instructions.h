#include <stdint.h>

#ifndef INSTRUCT_H
#define INSTRUCT_H

// BLOCK 0

//load instructions
void ld_r16_imm16(uint8_t r16);
void ld_R16MEM_a(uint8_t r16mem);
void ld_a_R16MEM(uint8_t r16mem);
void ld_IMM16_sp();

//r16 manipulation
void inc_r16(uint8_t r16);
void dec_r16(uint8_t r16);
void add_hl_r16(uint8_t r16);

//r8 manipulation
void inc_r8(uint8_t r8);
void dec_r8(uint8_t r8);

//r8 load
void ld_r8_imm8(uint8_t r8);

// bit manipulation
void rlca();
void rrca();
void rla();
void rra();
void cpl();
void scf();
void ccf();

//jumps
void jr_imm8();
void jr_cond_imm8(uint8_t cond);

//misc
void daa();
void nop();
void stop();

//BLOCK 1

void ld_r8_r8(uint8_t r8_1, uint8_t r8_2);
void halt();

//helpers
uint16_t *get_r16(uint8_t r16);
uint16_t *get_r16mem(uint8_t r16mem, int *offset);
uint8_t read_r8(uint8_t r8);
void write_r8(uint8_t r8, uint8_t val);
uint8_t is_cond_true(uint8_t cond);

#endif
