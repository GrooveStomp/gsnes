/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: mapper.h
  Created: 2019-11-04
  Updated: 2019-11-04
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file mapper.h
#include <stdint.h>

typedef bool (*map_cpu_read_fn)(void *interface, uint16_t addr, uint32_t *mapped_addr);

typedef bool (*map_cpu_write_fn)(void *interface, uint16_t addr, uint32_t *mapped_addr);

typedef bool (*map_ppu_read_fn)(void *interface, uint16_t addr, uint32_t *mapped_addr);

typedef bool (*map_ppu_write_fn)(void *interface, uint16_t addr, uint32_t *mapped_addr);

typedef void *(*mapper_init_fn)(uint8_t prg_banks, uint8_t chr_banks);

typedef void (*mapper_deinit_fn)(void *interface);
