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

//BLOCK 2

//8 bit arithmetic
void add_a_r8(uint8_t r8);
void adc_a_r8(uint8_t r8);
void sub_a_r8(uint8_t r8);
void sbc_a_r8(uint8_t r8);
void and_a_r8(uint8_t r8);
void xor_a_r8(uint8_t r8);
void or_a_r8(uint8_t r8);
void cp_a_r8(uint8_t r8);

//BLOCK 3

//8 bit arithmetic (immediate value)
void add_a_imm8();
void adc_a_imm8();
void sub_a_imm8();
void sbc_a_imm8();
void and_a_imm8();
void xor_a_imm8();
void or_a_imm8();
void cp_a_imm8();

//jumps and stack instructions
void ret_cond(uint8_t cond);
void ret();
void reti();
void jp_cond_imm16(uint8_t cond);
void jp_imm16();
void jp_hl();
void call_cond_imm16(uint8_t cond);
void call_imm16();
void rst_tgt3(uint8_t tgt3);

//stack push and pop
void pop_r16stk(uint16_t r16stk);
void push_r16stk(uint16_t r16stk);

//high RAM loads
void ldh_C_a();
void ldh_IMM8_a();
void ld_IMM16_a();
void ldh_a_C();
void ldh_a_IMM8();
void ld_a_IMM16();

//stack pointer manipulation
void add_sp_imm8();
void ld_hl_spPLUSimm8();
void ld_sp_hl();

//interrupt toggles
void di();
void ei();

//0xCB prefix register manipulation
void rlc_r8(uint8_t r8);
void rrc_r8(uint8_t r8);
void rl_r8(uint8_t r8);
void rr_r8(uint8_t r8);
void sla_r8(uint8_t r8);
void sra_r8(uint8_t r8);
void swap_r8(uint8_t r8);
void srl_r8(uint8_t r8);

//0xCB prefix bit manipulation
void bit_b3_r8(uint8_t b3, uint8_t r8);
void res_b3_r8(uint8_t b3, uint8_t r8);
void set_b3_r8(uint8_t b3, uint8_t r8);

//helpers
uint16_t *get_r16(uint8_t r16);
uint16_t *get_r16mem(uint8_t r16mem, int *offset);
uint8_t read_r8(uint8_t r8);
void write_r8(uint8_t r8, uint8_t val);
uint8_t is_cond_true(uint8_t cond);
uint16_t get_imm16();
uint16_t *get_r16stk(uint8_t r16);

#endif
