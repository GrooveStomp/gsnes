/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: ppu.h
  Created: 2019-11-03
  Updated: 2019-11-03
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

struct ppu;
struct cart;

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

#endif // PPU_VERSION
