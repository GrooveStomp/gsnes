/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: ppu.c
  Created: 2019-11-03
  Updated: 2019-12-03
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

//! CHR_ROM starts at 0x1000 == 4096 == 4K
static const int CHR_ROM = 0x1000;
static const int NAME_TABLE_SIZE = 1024;
static const int PATTERN_TABLE_SIZE = 4096;

union loopy_register {
        struct {
                uint16_t coarseX : 5;
                uint16_t coarseY : 5;
                uint16_t nametableX : 1;
                uint16_t nametableY : 1;
                uint16_t fineY : 3;
                uint16_t unused : 1;
        };
        uint16_t reg;
};

struct ppu {
        struct cart *cart;

        bool isFrameComplete;
        int16_t scanline; //!< Which row on the screen we are computing.
        int16_t cycle; //!< Which column on the screen we are computing.

        uint8_t **nameTables; //[2][1024];
        uint8_t **patternTables; //[2][4096];
        uint8_t *paletteTables; //[32];

        struct color *palette; //[0x40];
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

        union {
                struct {
                        uint8_t unused : 5;
			uint8_t spriteOverflow : 1;
			uint8_t spriteZeroHit : 1;
			uint8_t verticalBlank : 1;
                };
                uint8_t reg;
        } status;

        union {
                struct {
			uint8_t nametableX : 1;
			uint8_t nametableY : 1;
			uint8_t incrementMode : 1;
			uint8_t patternSprite : 1;
			uint8_t patternBackground : 1;
			uint8_t spriteSize : 1;
			uint8_t slaveMode : 1; // unused
			uint8_t enableNmi : 1;
                };
                uint8_t reg;
        } control;

        union loopy_register vramAddr;
        union loopy_register tramAddr;

        uint8_t fineX;

        uint8_t addressLatch;
        uint8_t dataBuffer;
        uint16_t address;

        uint8_t bgNextTileId;
        uint8_t bgNextTileAttrib;
        uint8_t bgNextTileLsb;
        uint8_t bgNextTileMsb;
        uint16_t bgShifterPatternLo;
        uint16_t bgShifterPatternHi;
        uint16_t bgShifterAttribLo;
        uint16_t bgShifterAttribHi;

        bool nmi;
};

struct ppu *PpuInit() {
        struct ppu *ppu = (struct ppu *)calloc(1, sizeof(struct ppu));
        if (NULL == ppu) {
                return NULL;
        }

        ppu->screen = SpriteInit(256, 240);
        if (NULL == ppu->screen) {
                PpuDeinit(ppu);
                return NULL;
        }

        uint8_t *nameTables = (uint8_t *)malloc(2 * NAME_TABLE_SIZE);
        if (NULL == nameTables) {
                PpuDeinit(ppu);
                return NULL;
        }
        ppu->nameTables = (uint8_t **)calloc(2, sizeof(uint8_t *));
        if (NULL == ppu->nameTables) {
                free(nameTables);
                PpuDeinit(ppu);
                return NULL;
        }
        ppu->nameTables[0] = &nameTables[0];
        ppu->nameTables[1] = &nameTables[NAME_TABLE_SIZE];

        uint8_t *patternTables = (uint8_t *)malloc(2 * PATTERN_TABLE_SIZE);
        if (NULL == patternTables) {
                PpuDeinit(ppu);
                return NULL;
        }
        ppu->patternTables = (uint8_t **)calloc(2, sizeof(uint8_t *));
        if (NULL == ppu->patternTables) {
                free(patternTables);
                PpuDeinit(ppu);
                return NULL;
        }
        ppu->patternTables[0] = &patternTables[0];
        ppu->patternTables[1] = &patternTables[PATTERN_TABLE_SIZE];

        ppu->paletteTables = (uint8_t *)calloc(32, sizeof(uint8_t));
        if (NULL == ppu->paletteTables) {
                PpuDeinit(ppu);
                return NULL;
        }

        ppu->nameTableSprites = (struct sprite **)calloc(2, sizeof(struct sprite *));
        if (NULL == ppu->nameTableSprites) {
                PpuDeinit(ppu);
                return NULL;
        }
        ppu->nameTableSprites[0] = SpriteInit(256, 240);
        if (NULL == ppu->nameTableSprites[0]) {
                free(ppu->nameTableSprites);
                ppu->nameTableSprites = NULL;
                PpuDeinit(ppu);
                return NULL;
        }
        ppu->nameTableSprites[1] = SpriteInit(256, 240);
        if (NULL == ppu->nameTableSprites[1]) {
                SpriteDeinit(ppu->nameTableSprites[0]);
                free(ppu->nameTableSprites);
                ppu->nameTableSprites = NULL;
                PpuDeinit(ppu);
                return NULL;
        }

        ppu->patternTableSprites = (struct sprite **)calloc(2, sizeof(struct sprite *));
        if (NULL == ppu->patternTableSprites) {
                PpuDeinit(ppu);
                return NULL;
        }
        ppu->patternTableSprites[0] = SpriteInit(128, 128);
        if (NULL == ppu->patternTableSprites[0]) {
                free(ppu->patternTableSprites);
                ppu->patternTableSprites = NULL;
                PpuDeinit(ppu);
                return NULL;
        }
        ppu->patternTableSprites[1] = SpriteInit(128, 128);
        if (NULL == ppu->patternTableSprites[1]) {
                SpriteDeinit(ppu->patternTableSprites[0]);
                free(ppu->patternTableSprites);
                ppu->patternTableSprites = NULL;
                PpuDeinit(ppu);
                return NULL;
        }

        ppu->cycle = 0;
        ppu->addressLatch = 0;
        ppu->dataBuffer = 0;
        ppu->address = 0;
        ppu->nmi = false;
        ppu->vramAddr.reg = 0;
        ppu->tramAddr.reg = 0;
        ppu->fineX = 0x00;
        ppu->bgNextTileId = 0;
        ppu->bgNextTileAttrib = 0;
        ppu->bgNextTileLsb = 0;
        ppu->bgNextTileMsb = 0;
        ppu->bgShifterPatternLo = 0;
        ppu->bgShifterPatternHi = 0;
        ppu->bgShifterAttribLo = 0;
        ppu->bgShifterAttribHi = 0;

        ppu->palette = (struct color *)calloc(0x40, sizeof(struct color));
        if (NULL == ppu->palette) {
                PpuDeinit(ppu);
                return NULL;
        }

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

        return ppu;
}

void PpuDeinit(struct ppu *ppu) {
        if (NULL == ppu) {
                return;
        }

        if (NULL != ppu->patternTables) {
                free(ppu->patternTables[0]);
                free(ppu->patternTables);
        }

        if (NULL != ppu->nameTables) {
                free(ppu->nameTables[0]);
                free(ppu->nameTables);
        }

        if (NULL != ppu->nameTableSprites) {
                SpriteDeinit(ppu->nameTableSprites[0]);
                SpriteDeinit(ppu->nameTableSprites[1]);
                free(ppu->nameTableSprites);
        }

        if (NULL != ppu->patternTableSprites) {
                SpriteDeinit(ppu->patternTableSprites[0]);
                SpriteDeinit(ppu->patternTableSprites[1]);
                free(ppu->patternTableSprites);
        }

        if (NULL != ppu->screen) {
                SpriteDeinit(ppu->screen);
        }

        if (NULL != ppu->paletteTables) {
                free(ppu->paletteTables);
        }

        if (NULL != ppu->palette) {
                free(ppu->palette);
        }

        free(ppu);
}

void PpuAttachCart(struct ppu *ppu, struct cart *cart) {
        ppu->cart = cart;
}

void IncrementScrollX(struct ppu *ppu) {
        if (!ppu->mask.renderBackground && !ppu->mask.renderSprites) return;

        if (31 == ppu->vramAddr.coarseX) {
                ppu->vramAddr.coarseX = 0;
                ppu->vramAddr.nametableX = ~ppu->vramAddr.nametableX;
        } else {
                ppu->vramAddr.coarseX++;
        }
}

void IncrementScrollY(struct ppu *ppu) {
        if (!ppu->mask.renderBackground && !ppu->mask.renderSprites) return;

        if (ppu->vramAddr.fineY < 7) {
                ppu->vramAddr.fineY++;
        } else {
                ppu->vramAddr.fineY = 0;

                if (29 == ppu->vramAddr.coarseY) {
                        ppu->vramAddr.coarseY = 0;
                        ppu->vramAddr.nametableY = ~ppu->vramAddr.nametableY;
                } else if (31 == ppu->vramAddr.coarseY) {
                        ppu->vramAddr.coarseY = 0;
                } else {
                        ppu->vramAddr.coarseY++;
                }
        }
}

void TransferAddressX(struct ppu *ppu) {
        if (!ppu->mask.renderBackground && !ppu->mask.renderSprites) return;

        ppu->vramAddr.nametableX = ppu->tramAddr.nametableX;
        ppu->vramAddr.coarseX = ppu->tramAddr.coarseX;
}

void TransferAddressY(struct ppu *ppu) {
        if (!ppu->mask.renderBackground && !ppu->mask.renderSprites) return;

        ppu->vramAddr.fineY = ppu->tramAddr.fineY;
        ppu->vramAddr.nametableY = ppu->tramAddr.nametableY;
        ppu->vramAddr.coarseY = ppu->tramAddr.coarseY;
}

void LoadBackgroundShifters(struct ppu *ppu) {
        // Each PPU update we calculate one pixel. These shifters shift 1 bit
        // along feeding the pixel compositor with the binary information it
        // needs. Its 16 bits wide, because the top 8 bits are the current 8
        // pixels being drawn and the bottom 8 bits are the next 8 pixels to be
        // drawn. Naturally this means the required bit is always the MSB of the
        // shifter. However, "fine x" scrolling plays a part in this too, whcih
        // is seen later, so in fact we can choose any one of the top 8 bits.
        ppu->bgShifterPatternLo = (ppu->bgShifterPatternLo & 0xFF00) | ppu->bgNextTileLsb;
        ppu->bgShifterPatternHi = (ppu->bgShifterPatternHi & 0xFF00) | ppu->bgNextTileMsb;

        // Attribute bits do not change per pixel, rather they change every 8
        // pixels but are synchronised with the pattern shifters for
        // convenience, so here we take the bottom 2 bits of the attribute word
        // which represent which palette is being used for the current 8 pixels
        // and the next 8 pixels, and "inflate" them to 8 bit words.
        ppu->bgShifterAttribLo = (ppu->bgShifterAttribLo & 0xFF00) | ((ppu->bgNextTileAttrib & 0x01) ? 0xFF : 0x00);
        ppu->bgShifterAttribHi = (ppu->bgShifterAttribHi & 0xFF00) | ((ppu->bgNextTileAttrib & 0x02) ? 0xFF : 0x00);
}

void UpdateShifters(struct ppu *ppu) {
        if (!ppu->mask.renderBackground) return;

        // Every cycle the shifters storing pattern and attribute information
        // shift their contents by 1 bit. This is because every cycle, the
        // output progresses by 1 pixel. This means relatively, the state of the
        // shifter is in sync with the pixels being drawn for that 8 pixel
        // section of the scanline.

        // Shift background tile pattern row.
        ppu->bgShifterPatternLo <<= 1;
        ppu->bgShifterPatternHi <<= 1;

        // Shift palette attributes by 1.
        ppu->bgShifterAttribLo <<= 1;
        ppu->bgShifterAttribHi <<= 1;
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
        if (ppu->scanline >= -1 && ppu->scanline < 240) {
                if (0 == ppu->scanline && 0 == ppu->cycle) {
                        ppu->cycle = 1;
                }

                // We've exited vblank/nmi and are ready to start rendering again.
                if (-1 == ppu->scanline && 1 == ppu->cycle) {
                        ppu->status.verticalBlank = 0;
                }

                if ((ppu->cycle >= 2 && ppu->cycle < 258) || (ppu->cycle >= 321 && ppu->cycle < 338)) {
                        UpdateShifters(ppu);

                        switch ((ppu->cycle - 1) % 8) {
                                case 0:
                                        LoadBackgroundShifters(ppu);

                                        // Fetch the next background tile ID.
                                        // 0x2000: nametable address space.
                                        // 0x0FFF: Mask to 12 bits
                                        ppu->bgNextTileId = PpuRead(ppu, 0x2000 | (ppu->vramAddr.reg & 0x0FFF));
                                        // Explanation:
                                        // The bottom 12 bits of the loopy
                                        // register provide an index into the 4
                                        // nametables, regardless of nametable
                                        // mirroring configuration.
                                        // nametable_y(1) nametable_x(1)
                                        // coarse_y(5) coarse_x(5)
                                        //
                                        // Consider a single nametable is a
                                        // 32x32 array, and we have four of them:
                                        //   0                1
                                        // 0 +----------------+----------------+
                                        //   |                |                |
                                        //   |                |                |
                                        //   |    (32x32)     |    (32x32)     |
                                        //   |                |                |
                                        //   |                |                |
                                        // 1 +----------------+----------------+
                                        //   |                |                |
                                        //   |                |                |
                                        //   |    (32x32)     |    (32x32)     |
                                        //   |                |                |
                                        //   |                |                |
                                        //   +----------------+----------------+
                                        //
                                        // This means there are 4096 potential
                                        // locations in this array, which
                                        // just so happens to be 2^12!
                                        break;

                                case 2: {
                                        // Fetch the next background tile
                                        // attribute.

                                        // Recall that each nametable has two
                                        // rows of cells that are not tile
                                        // information, instead they represent
                                        // the attribute information that
                                        // indicates which palettes are applied
                                        // to which area on the screen.
                                        // Importantly (and frustratingly) there
                                        // is not a 1 to 1 correspondance
                                        // between background tile and
                                        // palette. Two rows of tile data holds
                                        // 64 attributes. Therfore we can assume
                                        // that the attributes affect 8x8 zones
                                        // on the screen for that
                                        // nametable. Given a working resolution
                                        // of 256x240, we can further assume
                                        // that each zone is 32x32 pixels in
                                        // screen space, or 4x4 tiles. Four
                                        // system palettes are allocated to
                                        // background rendering, so a palette
                                        // can be specified using just 2
                                        // bits. The attribute byte therefore
                                        // can specify 4 distinct palettes.
                                        // Therefore we can even further assume
                                        // that a single palette is applied to a
                                        // 2x2 tile combination of the 4x4 tile
                                        // zone. The very fact that background
                                        // tiles "share" a palette locally is
                                        // the reason why in some games you see
                                        // distortion in the colours at screen
                                        // edges.

                                        // As before when choosing the tile ID,
                                        // we can use the bottom 12 bits of the
                                        // loopy register, but we need to make
                                        // the implementation "coarser" because
                                        // instead of a specific tile, we want
                                        // the attribute byte for a group of 4x4
                                        // tiles, or in other words, we divide
                                        // our 32x32 address by 4 to give us an
                                        // equivalent 8x8 address, and we offset
                                        // this address into the attribute
                                        // section of the target nametable.

                                        // Reconstruct the 12 bit loopy address
                                        // into an offset into the attribute
                                        // memory

                                        // "(vram_addr.coarse_x >> 2)"        : integer divide coarse x by 4,
                                        //                                      from 5 bits to 3 bits
                                        // "((vram_addr.coarse_y >> 2) << 3)" : integer divide coarse y by 4,
                                        //                                      from 5 bits to 3 bits,
                                        //                                      shift to make room for coarse x

                                        // Result so far: YX00 00yy yxxx

                                        // All attribute memory begins at 0x03C0
                                        // within a nametable, so OR with result
                                        // to select target nametable, and
                                        // attribute byte offset.
                                        uint16_t addr =
                                                0x23C0 |
                                                (ppu->vramAddr.nametableY << 11) |
                                                (ppu->vramAddr.nametableX << 10) |
                                                ((ppu->vramAddr.coarseY >> 2) << 3) |
                                                (ppu->vramAddr.coarseX >> 2);

                                        ppu->bgNextTileAttrib = PpuRead(ppu, addr);

                                        // The attribute byte is assembled thus:
                                        // BR(76) BL(54) TR(32) TL(10)
                                        //
                                        // +----+----+			    +----+----+
                                        // | TL | TR |			    | ID | ID |
                                        // +----+----+ where TL =   +----+----+
                                        // | BL | BR |			    | ID | ID |
                                        // +----+----+			    +----+----+
                                        //
                                        // Since we know we can access a tile
                                        // directly from the 12 bit address, we
                                        // can analyse the bottom bits of the
                                        // coarse coordinates to provide us with
                                        // the correct offset into the 8-bit
                                        // word, to yield the 2 bits we are
                                        // actually interested in which
                                        // specifies the palette for the 2x2
                                        // group of tiles. We know if "coarse y
                                        // % 4" < 2 we are in the top half else
                                        // bottom half.  Likewise if "coarse x %
                                        // 4" < 2 we are in the left half else
                                        // right half.  Ultimately we want the
                                        // bottom two bits of our attribute word
                                        // to be the palette selected. So shift
                                        // as required.
                                        if (ppu->vramAddr.coarseY & 0x02)
                                                ppu->bgNextTileAttrib >>= 4;

                                        if (ppu->vramAddr.coarseX & 0x02)
                                                ppu->bgNextTileAttrib >>= 2;

                                        ppu->bgNextTileAttrib &= 0x03;
                                        break;
                                }

                                case 4:
                                        // Fetch the next background tile LSB
                                        // bit plane from the pattern memory The
                                        // Tile ID has been read from the
                                        // nametable. We will use this id to
                                        // index into the pattern memory to find
                                        // the correct sprite (assuming the
                                        // sprites lie on 8x8 pixel boundaries
                                        // in that memory, which they do even
                                        // though 8x16 sprites exist, as
                                        // background tiles are always 8x8).
                                        //
                                        // Since the sprites are effectively 1
                                        // bit deep, but 8 pixels wide, we can
                                        // represent a whole sprite row as a
                                        // single byte, so offsetting into the
                                        // pattern memory is easy. In total
                                        // there is 8KB so we need a 13 bit
                                        // address.

                                        // "(control.pattern_background << 12)"  : the pattern memory selector
                                        //                                         from control register, either 0K
                                        //                                         or 4K offset
                                        // "((uint16_t)bg_next_tile_id << 4)"    : the tile id multiplied by 16, as
                                        //                                         2 lots of 8 rows of 8 bit pixels
                                        // "(vram_addr.fine_y)"                  : Offset into which row based on
                                        //                                         vertical scroll offset
                                        // "+ 0"                                 : Mental clarity for plane offset

                                        // Note: No PPU address bus offset
                                        // required as it starts at 0x0000
                                        ppu->bgNextTileLsb =
                                                PpuRead(ppu,
                                                        (ppu->control.patternBackground << 12) +
                                                        ((uint16_t)ppu->bgNextTileId << 4) +
                                                        (ppu->vramAddr.fineY) + 0);
                                        break;

                                case 6:
                                        // Fetch the next background tile MSB
                                        // bit plane from the pattern memory
                                        // This is the same as above, but has a
                                        // +8 offset to select the next bit
                                        // plane
                                        ppu->bgNextTileMsb =
                                                PpuRead(ppu,
                                                        (ppu->control.patternBackground << 12) +
                                                        ((uint16_t)ppu->bgNextTileId << 4) +
                                                        (ppu->vramAddr.fineY) + 8);
                                        break;

                                case 7:
                                        IncrementScrollX(ppu);
                                        break;
                        }
                }

                if (256 == ppu->cycle) {
                        IncrementScrollY(ppu);
                }

                if (257 == ppu->cycle) {
                        LoadBackgroundShifters(ppu);
                        TransferAddressX(ppu);
                }

                // Superflous reads of tile ID at end of scanline.
                if (338 == ppu->cycle || 340 == ppu->cycle) {
                        ppu->bgNextTileId = PpuRead(ppu, 0x2000 | (ppu->vramAddr.reg & 0x0FFF));
                }

                // End of vblank period, so reset the Y address.
                if (-1 == ppu->scanline && ppu->cycle >= 280 && ppu->cycle < 305) {
                        TransferAddressY(ppu);
                }
        }

        if (240 == ppu->scanline) {
                // Post render scanline - Do nothing!
        }

        if (ppu->scanline >= 241 && ppu->scanline < 261) {
                // We've entered vblank/nmi.
                if (241 == ppu->scanline && 1 == ppu->cycle) {
                        ppu->status.verticalBlank = 1;
                        if (ppu->control.enableNmi) ppu->nmi = true;
                }
        }

        uint8_t bgPixel = 0x00;
        uint8_t bgPalette = 0x00;

        if (ppu->mask.renderBackground) {
                uint16_t bitMux = 0x8000 >> ppu->fineX;

                uint8_t p0Pixel = (ppu->bgShifterPatternLo & bitMux) > 0;
                uint8_t p1Pixel = (ppu->bgShifterPatternHi & bitMux) > 0;

                bgPixel = (p1Pixel << 1) | p0Pixel;

                uint8_t bgPal0 = (ppu->bgShifterAttribLo & bitMux) > 0;
                uint8_t bgPal1 = (ppu->bgShifterAttribHi & bitMux) > 0;

                bgPalette = (bgPal1 << 1) | bgPal0;
        }

        struct color *color = PpuGetColorFromPaletteRam(ppu, bgPalette, bgPixel);
        SpriteSetPixel(ppu->screen, ppu->cycle - 1, ppu->scanline, color->rgba);

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

uint8_t PpuRead(struct ppu *ppu, uint16_t addr) {
        uint8_t data = 0x00;
        addr &= 0x3FFF; // 0x3FFFF is PPU base memory.

        if (CartPpuRead(ppu->cart, addr, &data)) {
        } else if (addr >= 0x0000 && addr <= 0x1FFF) { // Pattern Memory.
                // MSB determines which table.
                uint16_t pattern_index = (addr & 0x1000) >> 12;
                data = ppu->patternTables[pattern_index][addr & 0x0FFF];
        } else if (addr >= 0x2000 && addr <= 0x3EFF) {
                addr &= 0x0FFF;

                switch (CartMirroring(ppu->cart)) {
                        case MIRROR_VERTICAL: {
                                if (addr >= 0x0000 && addr <= 0x03FF)
                                        data = ppu->nameTables[0][addr & 0x03FF];
                                if (addr >= 0x0400 && addr <= 0x07FF)
                                        data = ppu->nameTables[1][addr & 0x03FF];
                                if (addr >= 0x0800 && addr <= 0x0BFF)
                                        data = ppu->nameTables[0][addr & 0x03FF];
                                if (addr >= 0x0C00 && addr <= 0x0FFF)
                                        data = ppu->nameTables[1][addr & 0x03FF];
                        } break;

                        case MIRROR_HORIZONTAL: {
                                if (addr >= 0x0000 && addr <= 0x03FF)
                                        data = ppu->nameTables[0][addr & 0x03FF];
                                if (addr >= 0x0400 && addr <= 0x07FF)
                                        data = ppu->nameTables[0][addr & 0x03FF];
                                if (addr >= 0x0800 && addr <= 0x0BFF)
                                        data = ppu->nameTables[1][addr & 0x03FF];
                                if (addr >= 0x0C00 && addr <= 0x0FFF)
                                        data = ppu->nameTables[1][addr & 0x03FF];
                        } break;

                        default: {

                        } break;
                }
        } else if (addr >= 0x3F00 && addr <= 0x3FFF) { // Palette Memory.
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
        } else if (addr >= 0x2000 && addr <= 0x3EFF) {
                addr &= 0x0FFF;

                switch (CartMirroring(ppu->cart)) {
                        case MIRROR_VERTICAL: {
                                if (addr >= 0x0000 && addr <= 0x03FF)
                                        ppu->nameTables[0][addr & 0x03FF] = data;
                                if (addr >= 0x0400 && addr <= 0x07FF)
                                        ppu->nameTables[1][addr & 0x03FF] = data;
                                if (addr >= 0x0800 && addr <= 0x0BFF)
                                        ppu->nameTables[0][addr & 0x03FF] = data;
                                if (addr >= 0x0C00 && addr <= 0x0FFF)
                                        ppu->nameTables[1][addr & 0x03FF] = data;
                        } break;

                        case MIRROR_HORIZONTAL: {
                                if (addr >= 0x0000 && addr <= 0x03FF)
                                        ppu->nameTables[0][addr & 0x03FF] = data;
                                if (addr >= 0x0400 && addr <= 0x07FF)
                                        ppu->nameTables[0][addr & 0x03FF] = data;
                                if (addr >= 0x0800 && addr <= 0x0BFF)
                                        ppu->nameTables[1][addr & 0x03FF] = data;
                                if (addr >= 0x0C00 && addr <= 0x0FFF)
                                        ppu->nameTables[1][addr & 0x03FF] = data;
                        } break;

                        default: {

                        } break;
                }
        } else if (addr >= 0x3F00 && addr <= 0x3FFF) { // Palette Memory.
                addr &= 0x001F; // Mask the bottom 5 bits.

                // Mirroring;
                if (addr == 0x0010) addr = 0x0000;
                if (addr == 0x0014) addr = 0x0004;
                if (addr == 0x0018) addr = 0x0008;
                if (addr == 0x001C) addr = 0x000C;
                ppu->paletteTables[addr] = data;
        }
}

void PpuWriteViaCpu(struct ppu *ppu, uint16_t addr, uint8_t data) {
        switch(addr) {
                case 0x0000: // Control
                        ppu->control.reg = data;
                        ppu->tramAddr.nametableX = ppu->control.nametableX;
                        ppu->tramAddr.nametableY = ppu->control.nametableY;
                break;

                case 0x0001: // Mask
                        ppu->mask.reg = data;
                break;

                case 0x0002: // Status
                break;

                case 0x0003: // OAM Address
                break;

                case 0x0004: // OAM Data
                break;

                case 0x0005: // Scroll
                        if (0 == ppu->addressLatch) {
                                ppu->fineX = data & 0x07;
                                ppu->tramAddr.coarseX = data >> 3;
                                ppu->addressLatch = 1;

                        } else {
                                ppu->tramAddr.fineY = data & 0x07;
                                ppu->tramAddr.coarseY = data >> 3;
                                ppu->addressLatch = 0;
                        }
                break;

                case 0x0006: // PPU Address
                        if (0 == ppu->addressLatch) {
                                ppu->tramAddr.reg = (uint16_t)((data & 0x3F) << 8) | (ppu->tramAddr.reg & 0x00FF);
                                ppu->addressLatch = 1;
                        } else {
                                ppu->tramAddr.reg = (ppu->tramAddr.reg & 0xFF00) | data;
                                ppu->vramAddr = ppu->tramAddr;
                                ppu->addressLatch = 0;
                        }
                break;

                case 0x0007: // PPU Data
                        PpuWrite(ppu, ppu->vramAddr.reg, data);
                        // Increment mode dictates whether we are writing data
                        // horizontally or vertically in memory.
                        ppu->vramAddr.reg += (ppu->control.incrementMode ? 32 : 1);
                break;
        }
}

uint8_t PpuReadViaCpu(struct ppu *ppu, uint16_t addr, bool readOnly) {
        uint8_t data = 0x00;

        // Reading from PPU registers can affect their contents so this read
        // only option is used for examining the state of the PPU without
        // changing its state. This is really only used in debug mode.
        if (readOnly) {
                switch(addr) {
                        case 0x0000: // Control
                                data = ppu->control.reg;
                                break;

                        case 0x0001: // Mask
                                data = ppu->mask.reg;
                                break;

                        case 0x0002: // Status
                                data = ppu->status.reg;
                                break;

                        case 0x0003: // OAM Address
                                break;

                        case 0x0004: // OAM Data
                                break;

                        case 0x0005: // Scroll
                                break;

                        case 0x0006: // PPU Address
                                break;

                        case 0x0007: // PPU Data
                                break;
                }
        }

        // These are the live PPU registers that repsond to being read from in
	// various ways. Note that not all the registers are capable of being
	// read from so they just return 0x00
        else { // Not ReadOnly
                switch (addr) {
                        case 0x000: // Control - Not Readable
                                break;

                        case 0x001: // Mask - Not Readable
                                break;

                        case 0x002: // Status
                                // Only the top 3 bits are actual status data.
                                // The remaining 5 bits are garbage - but _most_likely_
                                // whatever was in the data buffer.
                                data = (ppu->status.reg & 0xE0) | (ppu->dataBuffer & 0x1F);

                                // Reading status also clears the vertical blank flag.
                                ppu->status.verticalBlank = 0;

                                // Reading status also resets the address latch.
                                ppu->addressLatch = 0;
                                break;

                        case 0x003: // OAM Address
                                break;

                        case 0x004: // OAM Data
                                break;

                        case 0x005: // Scroll - Not Readable
                                break;

                        case 0x006: // PPU Address - Not Readable
                                break;

                        case 0x007: // PPU Data
                                // Normal reads are delayed by one cycle, so read the
                                // buffer, then refresh the buffer.
                                data = ppu->dataBuffer;
                                ppu->dataBuffer = PpuRead(ppu, ppu->vramAddr.reg);

                                // However, reading palette data is immediate.
                                if (ppu->vramAddr.reg >= 0x3F00) {
                                        data = ppu->dataBuffer;
                                }
                                ppu->vramAddr.reg += (ppu->control.incrementMode ? 32 : 1);
                                break;
                }
        }
        return data;
}


struct color *PpuGetColorFromPaletteRam(struct ppu *ppu, uint8_t palette, uint8_t pixel) {
        // Multiply the palette by 4 to get the physical offset.
        uint16_t palette_id = palette << 2;

        uint16_t addr = 0x3F00 + // Base address of palette memory.
                palette_id + // Which palette to read.
                pixel; // Offset of the specific color for this palette.

        // "& 0x3F" Stops read past the bounds of ppu->palette.
        return &ppu->palette[PpuRead(ppu, addr) & 0x3F];
}

struct sprite *PpuGetPatternTable(struct ppu *ppu, uint8_t i, uint8_t palette) {
        // Loop through all 16x16 tiles
        for (uint16_t tileY = 0; tileY < 16; tileY++) {
                for (uint16_t tileX = 0; tileX < 16; tileX++) {
                        // Convert the 2D tile coordinate into a 1D offset into pattern table memory.
                        uint16_t byteOffset = tileY * 256 + tileX * 16;

                        // Loop through 8 rows of 8 pixels per tile.
                        for (uint16_t row = 0; row < 8; row++) {
                                // Each pixel is 2 bits, stored in two separate bit planes.
                                // Each bit plane is 64 bits, which means the LSb
                                // and MSb are always 64 bits (8 bytes) apart.
                                uint8_t tileLsb = PpuRead(ppu, i * CHR_ROM + byteOffset + row + 0x0000);
                                uint8_t tileMsb = PpuRead(ppu, i * CHR_ROM + byteOffset + row + 0x0008);

                                // We read 8 bits worth of data, now we iterate
                                // through each column of the current row.
                                for (uint16_t col = 0; col < 8; col++) {
                                        uint8_t pixel = (tileLsb & 0x01) + (tileMsb & 0x01);

                                        // Shift each byte right one bit so the
                                        // next iteration works on the next
                                        // column.
                                        tileLsb >>= 1;
                                        tileMsb >>= 1;

                                        struct color *color = PpuGetColorFromPaletteRam(ppu, palette, pixel);

                                        // Because we are reading the LSb first,
                                        // we are effectively reading right to
                                        // left; so when we draw, we need to
                                        // make sure we are drawing in the right
                                        // order. Note that our index starts at
                                        // the left but our pixel is on the
                                        // right.
                                        int x = tileX * 8 + (7 - col);
                                        int y = tileY * 8 + row;

                                        SpriteSetPixel(ppu->patternTableSprites[i], x, y, color->rgba);
                                }
                        }
                }
        }

        return ppu->patternTableSprites[i];
}

void PpuReset(struct ppu *ppu) {
	ppu->fineX = 0x00;
	ppu->addressLatch = 0x00;
	ppu->dataBuffer = 0x00;
	ppu->scanline = 0;
	ppu->cycle = 0;
	ppu->bgNextTileId = 0x00;
	ppu->bgNextTileAttrib = 0x00;
	ppu->bgNextTileLsb = 0x00;
	ppu->bgNextTileMsb = 0x00;
	ppu->bgShifterPatternLo = 0x0000;
	ppu->bgShifterPatternHi = 0x0000;
	ppu->bgShifterAttribLo = 0x0000;
	ppu->bgShifterAttribHi = 0x0000;
	ppu->status.reg = 0x00;
	ppu->mask.reg = 0x00;
	ppu->control.reg = 0x00;
	ppu->vramAddr.reg = 0x0000;
	ppu->tramAddr.reg = 0x0000;
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

uint8_t PpuGetNmi(struct ppu *ppu) {
        return ppu->nmi;
}

void PpuSetNmi(struct ppu *ppu, uint8_t trueOrFalse) {
        ppu->nmi = trueOrFalse;
}

struct sprite *PpuGetNameTable(struct ppu *ppu, uint8_t i) {
        return ppu->nameTableSprites[i];
}
