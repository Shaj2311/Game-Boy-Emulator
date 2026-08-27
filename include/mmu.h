#ifndef MMU_H
#define MMU_H
#include <stdint.h>

uint8_t mmu_read(uint16_t addr);
void mmu_write(uint16_t addr, uint8_t val);

#endif
