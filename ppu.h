/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: ppu.h
  Created: 2019-11-03
  Updated: 2019-11-25
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file ppu.h
#ifndef PPU_VERSION
#define PPU_VERSION "0.1.0"

#include <stdint.h>

struct ppu;
struct cart;
struct sprite;
struct color;

struct ppu *
PpuInit();

void
PpuDeinit(struct ppu *ppu);

void
PpuAttachCart(struct ppu *ppu, struct cart *cart);

void
PpuTick(struct ppu *ppu);

void
PpuReset(struct ppu *ppu);

int
PpuIsFrameComplete(struct ppu *ppu);

void
PpuResetFrameCompletion(struct ppu *ppu);

struct sprite *
PpuScreen(struct ppu *ppu);

//! \brief Draws CHR ROM for a given pattern table into a sprite
//!
//! \param[in,out] ppu
//! \param[in] i which pattern table to draw
//! \param[in] palette which palette to use
//! \return a sprite representing CHR ROM
struct sprite *
PpuGetPatternTable(struct ppu *ppu, uint8_t i, uint8_t palette);

//! \brief Get the specified color from palette memory
//!
//! \param[in,out] ppu
//! \param[in] palette which palette to use for color
//! \param[in] pixel 0, 1, 2 or 3
struct color *
PpuGetColorFromPaletteRam(struct ppu *ppu, uint8_t palette, uint8_t pixel);

uint8_t
PpuReadViaCpu(struct ppu *ppu, uint16_t addr);

void
PpuWriteViaCpu(struct ppu *ppu, uint16_t addr, uint8_t data);

uint8_t
PpuRead(struct ppu *ppu, uint16_t addr);

void
PpuWrite(struct ppu *ppu, uint16_t addr, uint8_t data);

uint8_t
PpuGetNmi(struct ppu *ppu);

void
PpuSetNmi(struct ppu *ppu, uint8_t trueOrFalse);

uint8_t *
PpuGetNameTable(struct ppu *ppu, uint8_t i);

#endif // PPU_VERSION
