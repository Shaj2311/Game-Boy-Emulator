#include <string.h>

#include "ppu.h"
#include "addr.h"

void ppu_set_mode(PPU_Mode mode)
{
	uint8_t oldMode = gb.ppu_mode;
	gb.ppu_mode = mode;
	gb.sysbus[STAT_ADDR] = (gb.sysbus[STAT_ADDR] & 0xFC) | mode;

	//request STAT interrupt on mode transition (except pixel transfer mode)
	uint8_t STAT_curr_int = getSTATint();
	if(gb.STAT_old_int == 0 && STAT_curr_int == 1)
	{
		gb.sysbus[0xFF0F] |= 0x02;
	}
	gb.STAT_old_int = STAT_curr_int;

	//execute current PPU mode
	if(gb.ppu_mode != oldMode)
	{
		if(gb.ppu_mode == PPU_MODE_OAM_SEARCH)
			gb.OAM_search_result = ppu_oam_search();
		else if(gb.ppu_mode == PPU_MODE_PIX_TRANS)
			ppu_pixel_transfer();
	}
}

void ppu_set_LY(uint8_t LY)
{
	//write new scanline to LY
	gb.sysbus[LY_ADDR] = LY;

	//update LYC==LY
	if(gb.sysbus[LYC_ADDR] == LY)
	{
		gb.sysbus[STAT_ADDR] |= 0x04;
		//request STAT interrupt
		uint8_t STAT_curr_int = getSTATint();
		if(gb.STAT_old_int == 0 && STAT_curr_int == 1)
		{
			gb.sysbus[0xFF0F] |= 0x02;
		}
		gb.STAT_old_int = STAT_curr_int;
	}
	else
		gb.sysbus[STAT_ADDR] &= ~(0X04);
}

void ppu_timer_tick()
{
	//get LCD control
	uint8_t LCDC = gb.sysbus[0xFF40];
	//check LDC and PPU enable
	if(!(LCDC >> 7))
	{
		//reset cycles
		gb.ppu_cycles = 0;

		//reset mode
		ppu_set_mode(PPU_MODE_HBLANK);

		//reset to top scanline
		ppu_set_LY(0);

		return;
	}

	gb.ppu_cycles++; //tick by 1 cycle
	uint8_t LY = gb.sysbus[LY_ADDR];
	if(LY < 144)
	{
		if(gb.ppu_cycles <= 79)
		{
			//OAM search
			ppu_set_mode(PPU_MODE_OAM_SEARCH);
		}
		else if(gb.ppu_cycles <= 251)
		{
			//Pixel transfer
			ppu_set_mode(PPU_MODE_PIX_TRANS);
		}
		else if(gb.ppu_cycles <= 455)
		{
			//H-blank
			ppu_set_mode(PPU_MODE_HBLANK);
		}
		else
		{
			//End of line reached

			//move to next line
			gb.ppu_cycles -= 456;
			ppu_set_LY(++LY);

			//check vblank
			if(LY == 144)
			{
				//Vblank
				ppu_set_mode(PPU_MODE_VBLANK);
				//request vblank interrupt
				gb.sysbus[0xFF0F] |= 0x01;
			}
			else
			{
				//OAM Search
				ppu_set_mode(PPU_MODE_OAM_SEARCH);
			}
		}
	}
	else
	{
		//vblank
		ppu_set_mode(PPU_MODE_VBLANK);

		//visible scanlines complete
		if(gb.ppu_cycles >= 456)
		{
			gb.ppu_cycles -= 456;

			//if all scanlines complete (including hidden), reset to line 0
			if(LY == 153)
			{
				//reset ppu mode
				ppu_set_mode(PPU_MODE_OAM_SEARCH);

				//reset rendered window lines count
				gb.windowLinesRendered = 0;

				//reset to line 0
				ppu_set_LY(0);
			}
			else
			{
				//move to next line
				ppu_set_LY(++LY);
			}
		}
	}
}

OAM_Result ppu_oam_search()
{
	OAM_Result result;
	result.count = 0;

	uint8_t LCDC = gb.sysbus[LCDC_ADDR];

	//check OBJ enable
	if(!(LCDC & 0x02))
		return result;

	uint8_t spriteHeight = LCDC & 0x04 ? 16 : 8;

	//get LY
	uint8_t LY = gb.sysbus[LY_ADDR];

	//search OAM
	uint16_t addr = 0xFE00;
	while(result.count < 10 && addr <= 0xFE9F)
	{
		//get sprite
		Sprite *ptr = (Sprite *)(gb.sysbus + addr);
		//compare Y with LY
		if(LY + 16 >= ptr->y && LY + 16 < ptr->y + spriteHeight)
			result.sprites[result.count++] = ptr;

		addr += sizeof(Sprite);
	}

	return result;
}

void ppu_pixel_transfer()
{
	//get LCD control
	uint8_t LCDC = gb.sysbus[LCDC_ADDR];

	//get current scanline, set up X and Y
	uint8_t Y = gb.sysbus[LY_ADDR];
	uint8_t X = 0;

	//get OAM search result
	OAM_Result oamSearchResult = gb.OAM_search_result;

	//check LCD & PPU enable
	if(!(LCDC & 0x80))
	{
		//reset screen to white
		uint32_t whiteRGBA = ppu_lookup_RGBA(0, gb.sysbus[BGP_ADDR]);
		for(int x = 0; x < 160; x++)
			gb.frameBuffer[Y * 160 + x] = whiteRGBA;
		return;
	}

	//set up scanline information buffers
	uint8_t currBg[160];

	//get scroll position (SCY, SCX)
	uint8_t SCY = gb.sysbus[SCY_ADDR];
	uint8_t SCX = gb.sysbus[SCX_ADDR];

	//get window position (WY, WX)
	uint8_t WY = gb.sysbus[WY_ADDR];
	uint8_t WX = gb.sysbus[WX_ADDR];

	//get background tilemap starting address
	uint16_t bgTileMapAddr = (LCDC >> 3) & 0x01 ? 0x9C00 : 0x9800;

	//get window tilemap starting address
	uint16_t winTileMapAddr = (LCDC >> 6) & 0x01 ? 0x9C00 : 0x9800;

	//check BG & Window enable
	if((LCDC & 0x01))
	{
		//render background
		ppu_pix_trans_bg(X, Y, SCX, SCY, LCDC, bgTileMapAddr, currBg);

		//render window
		ppu_pix_trans_win(X, Y, WX, WY, LCDC, winTileMapAddr, currBg);
	}
	else
	{
		//don't draw background
		memset(currBg, 0, 160);
	}

	//draw bg/window colors to framebuffer
	uint8_t BGP = gb.sysbus[BGP_ADDR];
	for(int x = 0; x < 160; x++)
	{
		gb.frameBuffer[Y * 160 + x] = ppu_lookup_RGBA(currBg[x], BGP);
	}

	//render sprites
	ppu_pix_trans_sprites(oamSearchResult, LCDC, currBg);
}

void ppu_pix_trans_bg(uint8_t X, uint8_t Y, uint8_t SCX, uint8_t SCY, uint8_t LCDC, uint16_t bgTileMapAddr, uint8_t *currBg)
{
	//render background
	while(X < 160)
	{
		//convert screen coordinates to canvas coordinates
		uint8_t canvasX = X + SCX;
		uint8_t canvasY = Y + SCY;

		//get tile index
		uint16_t tilemapEntryAddr =
			bgTileMapAddr
			+ ((canvasY / 8) * 32)
			+ (canvasX / 8);

		uint8_t tileIndex = gb.sysbus[tilemapEntryAddr];

		//get 2bpp data
		uint8_t currTileBit = canvasY % 8;
		uint16_t tileDataAddr;

		if(LCDC & 0x10) //address depends on signed vs unsigned addressing
			tileDataAddr = 0x8000 + (tileIndex * 16) + (currTileBit * 2);
		else
			tileDataAddr = 0x9000 + ((int8_t)tileIndex * 16) + (currTileBit * 2);

		uint8_t lowByte = gb.sysbus[tileDataAddr];
		uint8_t highByte = gb.sysbus[tileDataAddr + 1];

		//extract and store 2bpp data
		uint8_t bitPosition = 7 - (canvasX % 8);
		uint8_t highBit = (highByte >> bitPosition) & 0x01;
		uint8_t lowBit = (lowByte >> bitPosition) & 0x01;
		uint8_t colorIndex = (highBit << 1) | lowBit;
		currBg[X] = colorIndex;

		//advance through scanline
		X++;
	}
}

void ppu_pix_trans_win(uint8_t X, uint8_t Y, uint8_t WX, uint8_t WY, uint8_t LCDC, uint16_t winTileMapAddr, uint8_t *currBg)
{
	//check window enable
	if(!(LCDC & 0x20))
		return;

	//check position bounds
	if(Y < WY || WX >= (160 + 6))
		return;

	int windowXoffset = (int)WX - 7;
	uint8_t lineRendered = 0;

	while(X < 160)
	{
		if((int)X < windowXoffset)
		{
			X++;
			continue;
		}

		lineRendered = 1;

		//convert screen coordinates to canvas coordinates
		uint8_t canvasX = (int)X - windowXoffset;
		uint8_t canvasY = gb.windowLinesRendered;

		//get tile index
		uint16_t tilemapEntryAddr =
			winTileMapAddr
			+ ((canvasY / 8) * 32)
			+ (canvasX / 8);

		uint8_t tileIndex = gb.sysbus[tilemapEntryAddr];

		//get 2bpp data
		uint8_t currTileBit = canvasY % 8;
		uint16_t tileDataAddr;

		if(LCDC & 0x10) //address depends on signed vs unsigned addressing
			tileDataAddr = 0x8000 + (tileIndex * 16) + (currTileBit * 2);
		else
			tileDataAddr = 0x9000 + ((int8_t)tileIndex * 16) + (currTileBit * 2);

		uint8_t lowByte = gb.sysbus[tileDataAddr];
		uint8_t highByte = gb.sysbus[tileDataAddr + 1];

		//extract and store 2bpp data
		uint8_t bitPosition = 7 - (canvasX % 8);
		uint8_t highBit = (highByte >> bitPosition) & 0x01;
		uint8_t lowBit = (lowByte >> bitPosition) & 0x01;
		uint8_t colorIndex = (highBit << 1) | lowBit;
		currBg[X] = colorIndex;

		//advance through scanline
		X++;
	}

	if(lineRendered)
		gb.windowLinesRendered++;
}

void ppu_pix_trans_sprites(OAM_Result OAM_sprites, uint8_t LCDC, uint8_t *currBg)
{
	//check object enable
	if(!(LCDC & 0x02))
		return;

	Sprite **sprites = OAM_sprites.sprites;
	uint8_t LY = gb.sysbus[LY_ADDR];

	//for each screen pixel on current scanline,
	for(int i = 0; i < 160; i++)
	{
		//search for best sprite
		uint16_t bestX = -1;
		uint8_t bestColorIndex = 0;
		Sprite *bestSprite = 0;
		for(int j = 0; j < OAM_sprites.count; j++)
		{
			//check if current pixel falls within sprite's horizontal bounds
			int16_t spriteX = sprites[j]->x - 8;
			if(!(spriteX <= i && i <= spriteX + 7))
				continue;

			//get tile index, reset bit 0 in case of 8x16 mode
			uint8_t tileIndex = sprites[j]->tileIndex;
			uint8_t spriteHeight = LCDC & 0x04 ? 16 : 8;
			if(spriteHeight == 16)
				tileIndex &= 0xFE;

			//get row inside tile that current pixel is in
			uint8_t tileY = LY - (sprites[j]->y - 16);

			//check vertical flipping (Y-flip)
			if(sprites[j]->attr & 0x40)
				tileY = (spriteHeight - 1) - tileY;

			//get 2bpp
			uint16_t tileDataAddr = 0x8000 + (tileIndex * 16) + (tileY * 2);
			uint8_t lowByte = gb.sysbus[tileDataAddr];
			uint8_t highByte = gb.sysbus[tileDataAddr + 1];

			//check horizontal flip (X-flip)
			uint8_t tileX = i - spriteX;
			uint8_t bitPosition = (sprites[j]->attr & 0x20) ? tileX : (7 - tileX);

			//extract 2bpp data
			uint8_t highBit = (highByte >> bitPosition) & 0x01;
			uint8_t lowBit = (lowByte >> bitPosition) & 0x01;
			uint8_t colorIndex = (highBit << 1) | lowBit;

			//check transparency of pixel
			if(!colorIndex)
				continue;

			//compare with best X
			if(sprites[j]->x < bestX)
			{
				bestX = sprites[j]->x;
				bestSprite = sprites[j];
				bestColorIndex = colorIndex;
			}
		}

		//if no sprites were overlapping on this pixel, skip
		if(!bestSprite)
			continue;

		//check priority
		uint8_t priority = bestSprite->attr >> 7;
		uint8_t currBgColor = currBg[i];
		//if background is enabled and current pixel's bg color has priority over sprite, skip drawing sprite
		if((LCDC & 0x01) && priority && (currBgColor != 0))
			continue;

		//write pixel to framebuffer
		uint8_t palette = bestSprite->attr & 0x10 ? gb.sysbus[OBP1_ADDR] : gb.sysbus[OBP0_ADDR];
		gb.frameBuffer[LY * 160 + i] = ppu_lookup_RGBA(bestColorIndex, palette);
	}
}

uint32_t ppu_lookup_RGBA(uint8_t colorIndex, uint8_t paletteReg)
{
	uint8_t shadeIndex = (paletteReg >> (colorIndex * 2)) & 0x03;
	return RGBA[shadeIndex];
}

