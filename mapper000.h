/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: mapper000.h
  Created: 2019-11-04
  Updated: 2019-11-04
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
//! \file mapper000.h

struct mapper000;

void *
Mapper000_Init(uint8_t prg_banks, uint8_t chr_banks);

void
Mapper000_Deinit(void *mapper);

bool
Mapper000_MapCpuRead(void *mapper, uint16_t addr, uint32_t *mapped_addr);

bool
Mapper000_MapCpuWrite(void *mapper, uint16_t addr, uint32_t *mapped_addr);

bool
Mapper000_MapPpuRead(void *mapper, uint16_t addr, uint32_t *mapped_addr);

bool
Mapper000_MapPpuWrite(void *mapper, uint16_t addr, uint32_t *mapped_addr);
