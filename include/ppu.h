#ifndef PPU_H
#define PPU_H
#include "gb.h"

void ppu_set_mode(PPU_Mode mode);
void ppu_set_LY(uint8_t LY);
OAM_Result ppu_oam_search();
void ppu_pixel_transfer();
void ppu_pix_trans_bg(uint8_t X, uint8_t Y, uint8_t SCX, uint8_t SCY, uint8_t LCDC, uint16_t bgTileMapAddr, uint8_t *currBg);
void ppu_pix_trans_win(uint8_t X, uint8_t Y, uint8_t WX, uint8_t WY, uint8_t LCDC, uint16_t winTileMapAddr, uint8_t *currBg);
void ppu_pix_trans_sprites(OAM_Result sprites, uint8_t LCDC, uint8_t *currBg);
uint32_t ppu_lookup_RGBA(uint8_t code, uint8_t paletteReg);

#endif
