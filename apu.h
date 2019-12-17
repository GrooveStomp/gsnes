/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: apu.h
  Created: 2019-12-17
  Updated: 2019-12-17
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file apu.h
#include <stdint.h>

struct apu;

struct apu *
ApuInit();

void
ApuDeinit(struct apu *apu);

void
ApuWrite(struct apu *apu, uint16_t addr, uint8_t data);

uint8_t
ApuRead(struct apu *apu, uint16_t addr);

void
ApuTick(struct apu *apu);

void
ApuReset(struct apu *apu);

double
ApuGetOutputSample(struct apu *apu);

uint8_t
ApuDidLastTickProduceSample(struct apu *apu);
