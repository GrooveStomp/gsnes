/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: ppu.c
  Created: 2019-11-03
  Updated: 2019-11-05
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file ppu.c
#include <stdlib.h> // malloc, free
#include <stdbool.h> // bool
#include <stdio.h>

#include "ppu.h"
#include "cart.h"
#include "color.h"
#include "sprite.h"

struct ppu {
        struct cart *cart;

        bool isFrameComplete;
        int16_t scanline; //!< Which row on the screen we are computing.
        int16_t cycle; //!< Which column on the screen we are computing.

        uint8_t **nameTables; //[2][1024];
        uint8_t **patternTables; //[2][4096];
        uint8_t *paletteTables; //[32];

        struct color *palette;
        struct sprite *screen;
        struct sprite **nameTableSprites;
        struct sprite **patternTableSprites;

        union {
                struct {
                        uint8_t grayscale : 1;
                        uint8_t renderBackgroundLeft : 1;
                        uint8_t renderSpritesLeft : 1;
                        uint8_t renderBackground : 1;
                        uint8_t renderSprites : 1;
                        uint8_t enhanceRed : 1;
                        uint8_t enhanceGreen : 1;
                        uint8_t enhanceBlue : 1;
                };
                uint8_t reg;

        } mask;
};

struct ppu *PpuInit() {
        struct ppu *ppu = (struct ppu *)malloc(sizeof(struct ppu));
        if (NULL == ppu) {
                return NULL;
        }

        ppu->palette = (struct color *)calloc(0x40, sizeof(struct color));
        if (NULL == ppu->palette) {
                PpuDeinit(ppu);
                return NULL;
        }

        ppu->screen = SpriteInit(256, 240);
        if (NULL == ppu->screen) {
                PpuDeinit(ppu);
                return NULL;
        }

        ppu->nameTables = (uint8_t **)calloc(2, 1024);
        if (NULL == ppu->nameTables) {
                PpuDeinit(ppu);
                return NULL;
        }

        ppu->patternTables = (uint8_t **)calloc(2, 4096);
        if (NULL == ppu->patternTables) {
                PpuDeinit(ppu);
                return NULL;
        }

        ppu->paletteTables = (uint8_t *)calloc(32, 1);
        if (NULL == ppu->paletteTables) {
                PpuDeinit(ppu);
                return NULL;
        }

        ppu->nameTableSprites = (struct sprite **)calloc(2, sizeof(struct sprite *));
        ppu->nameTableSprites[0] = SpriteInit(256, 240);
        ppu->nameTableSprites[1] = SpriteInit(256, 240);
        // TODO: Error handling

        ppu->patternTableSprites = (struct sprite **)calloc(2, sizeof(struct sprite *));
        ppu->patternTableSprites[0] = SpriteInit(128, 128);
        ppu->patternTableSprites[1] = SpriteInit(128, 128);
        // TODO: Error handling

        ppu->palette[0x00] = ColorInitInts(84, 84, 84, 255);
	ppu->palette[0x01] = ColorInitInts(0, 30, 116, 255);
	ppu->palette[0x02] = ColorInitInts(8, 16, 144, 255);
	ppu->palette[0x03] = ColorInitInts(48, 0, 136, 255);
	ppu->palette[0x04] = ColorInitInts(68, 0, 100, 255);
	ppu->palette[0x05] = ColorInitInts(92, 0, 48, 255);
	ppu->palette[0x06] = ColorInitInts(84, 4, 0, 255);
	ppu->palette[0x07] = ColorInitInts(60, 24, 0, 255);
	ppu->palette[0x08] = ColorInitInts(32, 42, 0, 255);
	ppu->palette[0x09] = ColorInitInts(8, 58, 0, 255);
	ppu->palette[0x0A] = ColorInitInts(0, 64, 0, 255);
	ppu->palette[0x0B] = ColorInitInts(0, 60, 0, 255);
	ppu->palette[0x0C] = ColorInitInts(0, 50, 60, 255);
	ppu->palette[0x0D] = ColorInitInts(0, 0, 0, 255);
	ppu->palette[0x0E] = ColorInitInts(0, 0, 0, 255);
	ppu->palette[0x0F] = ColorInitInts(0, 0, 0, 255);

	ppu->palette[0x10] = ColorInitInts(152, 150, 152, 255);
	ppu->palette[0x11] = ColorInitInts(8, 76, 196, 255);
	ppu->palette[0x12] = ColorInitInts(48, 50, 236, 255);
	ppu->palette[0x13] = ColorInitInts(92, 30, 228, 255);
	ppu->palette[0x14] = ColorInitInts(136, 20, 176, 255);
	ppu->palette[0x15] = ColorInitInts(160, 20, 100, 255);
	ppu->palette[0x16] = ColorInitInts(152, 34, 32, 255);
	ppu->palette[0x17] = ColorInitInts(120, 60, 0, 255);
	ppu->palette[0x18] = ColorInitInts(84, 90, 0, 255);
	ppu->palette[0x19] = ColorInitInts(40, 114, 0, 255);
	ppu->palette[0x1A] = ColorInitInts(8, 124, 0, 255);
	ppu->palette[0x1B] = ColorInitInts(0, 118, 40, 255);
	ppu->palette[0x1C] = ColorInitInts(0, 102, 120, 255);
	ppu->palette[0x1D] = ColorInitInts(0, 0, 0, 255);
	ppu->palette[0x1E] = ColorInitInts(0, 0, 0, 255);
	ppu->palette[0x1F] = ColorInitInts(0, 0, 0, 255);

	ppu->palette[0x20] = ColorInitInts(236, 238, 236, 255);
	ppu->palette[0x21] = ColorInitInts(76, 154, 236, 255);
	ppu->palette[0x22] = ColorInitInts(120, 124, 236, 255);
	ppu->palette[0x23] = ColorInitInts(176, 98, 236, 255);
	ppu->palette[0x24] = ColorInitInts(228, 84, 236, 255);
	ppu->palette[0x25] = ColorInitInts(236, 88, 180, 255);
	ppu->palette[0x26] = ColorInitInts(236, 106, 100, 255);
	ppu->palette[0x27] = ColorInitInts(212, 136, 32, 255);
	ppu->palette[0x28] = ColorInitInts(160, 170, 0, 255);
	ppu->palette[0x29] = ColorInitInts(116, 196, 0, 255);
	ppu->palette[0x2A] = ColorInitInts(76, 208, 32, 255);
	ppu->palette[0x2B] = ColorInitInts(56, 204, 108, 255);
	ppu->palette[0x2C] = ColorInitInts(56, 180, 204, 255);
	ppu->palette[0x2D] = ColorInitInts(60, 60, 60, 255);
	ppu->palette[0x2E] = ColorInitInts(0, 0, 0, 255);
	ppu->palette[0x2F] = ColorInitInts(0, 0, 0, 255);

	ppu->palette[0x30] = ColorInitInts(236, 238, 236, 255);
	ppu->palette[0x31] = ColorInitInts(168, 204, 236, 255);
	ppu->palette[0x32] = ColorInitInts(188, 188, 236, 255);
	ppu->palette[0x33] = ColorInitInts(212, 178, 236, 255);
	ppu->palette[0x34] = ColorInitInts(236, 174, 236, 255);
	ppu->palette[0x35] = ColorInitInts(236, 174, 212, 255);
	ppu->palette[0x36] = ColorInitInts(236, 180, 176, 255);
	ppu->palette[0x37] = ColorInitInts(228, 196, 144, 255);
	ppu->palette[0x38] = ColorInitInts(204, 210, 120, 255);
	ppu->palette[0x39] = ColorInitInts(180, 222, 120, 255);
	ppu->palette[0x3A] = ColorInitInts(168, 226, 144, 255);
	ppu->palette[0x3B] = ColorInitInts(152, 226, 180, 255);
	ppu->palette[0x3C] = ColorInitInts(160, 214, 228, 255);
	ppu->palette[0x3D] = ColorInitInts(160, 162, 160, 255);
	ppu->palette[0x3E] = ColorInitInts(0, 0, 0, 255);
	ppu->palette[0x3F] = ColorInitInts(0, 0, 0, 255);

        ppu->cycle = 0;
        return ppu;
}

void PpuDeinit(struct ppu *ppu) {
        if (NULL == ppu) {
                return;
        }

        free(ppu);
}

void PpuAttachCart(struct ppu *ppu, struct cart *cart) {
        ppu->cart = cart;
}

//! \brief advance the renderer one pixel across the screen
//!
//! Advance the pixel right one pixel on the current row, or to the start of the
//! next row if we've reached the screen edge.
//! When the ppu has reached the end of the screen entirely, set
//! isFrameComplete to true.
//!
//! \param[in,out] ppu
void PpuTick(struct ppu *ppu) {
        // For now we are just drawing random noise.
        uint16_t idx = (rand() % 2) ? 0x3F : 0x30;
        struct color color = ppu->palette[idx];
        SpriteSetPixel(ppu->screen, ppu->cycle - 1, ppu->scanline, color.rgba);

        ppu->cycle++;

        if (341 < ppu->cycle) {
                ppu->cycle = 0;
                ppu->scanline++;

                if (261 < ppu->scanline) {
                        ppu->scanline = -1;
                        ppu->isFrameComplete = true;
                }
        }
}

struct sprite *PpuGetPatternTable(struct ppu *ppu, uint8_t i) {
        return ppu->patternTableSprites[i];
}

uint8_t PpuRead(struct ppu *ppu, uint16_t addr) {
        uint8_t data = 0x00;
        addr &= 0x3FFF; // 0x3FFFF is PPU base memory.

        if (CartPpuRead(ppu->cart, addr, &data)) {
        } else if (addr >= 0x0000 && addr <= 0x1FFF) { // Pattern Memory.
                // MSB determines which table.
                uint16_t pattern_index = (addr & 0x1000) >> 12;
                data = ppu->patternTables[pattern_index][addr & 0x0FFF];
        } else if (addr >= 0x2000 && addr <= 0x2FFF) {
        } else if (addr >= 0x3000 && addr <= 0x3FFF) { // Palette Memory.
                addr &= 0x001F; // Mask the bottom 5 bits.

                // Mirroring;
                if (addr == 0x0010) addr = 0x0000;
                if (addr == 0x0014) addr = 0x0004;
                if (addr == 0x0018) addr = 0x0008;
                if (addr == 0x001C) addr = 0x000C;
                data = ppu->paletteTables[addr] & (ppu->mask.grayscale ? 0x30 : 0x3F);
        }

        return data;
}

void PpuWrite(struct ppu *ppu, uint16_t addr, uint8_t data) {
        addr &= 0x3FFF; // 0x3FFFF is PPU base memory.

        if (CartPpuWrite(ppu->cart, addr, data)) {
        } else if (addr >= 0x0000 && addr <= 0x1FFF) { // Pattern Memory.
                // Pattern memory is _usually_ a ROM, but we support writes here
                // as well.

                // MSB determines which table.
                uint16_t pattern_index = (addr & 0x1000) >> 12;
                ppu->patternTables[pattern_index][addr & 0x0FFF] = data;
        } else if (addr >= 0x2000 && addr <= 0x2FFF) {
        } else if (addr >= 0x3000 && addr <= 0x3FFF) { // Palette Memory.
                addr &= 0x001F; // Mask the bottom 5 bits.

                // Mirroring;
                if (addr == 0x0010) addr = 0x0000;
                if (addr == 0x0014) addr = 0x0004;
                if (addr == 0x0018) addr = 0x0008;
                if (addr == 0x001C) addr = 0x000C;
                ppu->paletteTables[addr] = data;
        }
}

//! \brief Get the specified color from palette memory
//!
//! \param palette which palette to use for color
//! \param pixel 0, 1, 2 or 3
struct color *GetColorFromPaletteRam(struct ppu *ppu, uint8_t palette, uint8_t pixel) {
        uint16_t palette_id = palette << 2; // Multiply the palette by 4 to get the
                                   // physical offset.

        uint16_t addr = 0x3F00 + // Base address of palette memory.
                palette_id + // Which palette to read.
                pixel; // Offset of the specific color for this palette.

        // "& 0x3F" Stops read past the bounds of ppu->palette.
        return &ppu->palette[PpuRead(ppu, addr) & 0x3F];
}

void PpuReset(struct ppu *ppu) {
        // TODO PpuReset()
}

int PpuIsFrameComplete(struct ppu *ppu) {
        return ppu->isFrameComplete;
}

void PpuResetFrameCompletion(struct ppu *ppu) {
        ppu->isFrameComplete = false;
}

struct sprite *PpuScreen(struct ppu *ppu) {
        return ppu->screen;
}
