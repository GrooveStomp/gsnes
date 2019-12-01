/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: mapper.h
  Created: 2019-11-04
  Updated: 2019-11-27
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file mapper.h
//! This file describes the `mapper` interface.
//! There are seven function pointers defined that must be provided by any
//! concrete implementation of this interface.
#include <stdint.h>

//! \brief Initialize the mapper
//! \return Pointer to mapper
typedef void *(*mapper_init_fn)(uint8_t prgBanks, uint8_t chrBanks);

//! \brief De-initialize the mapper
//! \param[in,out] interface the mapper
typedef void (*mapper_deinit_fn)(void *interface);

//! \brief Reset the mapper
//! \param[in,out] interface the mapper
typedef void (*mapper_reset_fn)(void *interface);

//! \brief CPU read intercept
//! \param[in,out] interface the mapper
//! \param[in] addr 16-bit address to read
//! \param[out] mappedAddr the translated address
typedef bool (*map_cpu_read_fn)(void *interface, uint16_t addr, uint32_t *mappedAddr);

//! \brief CPU write intercept
//! \param[in,out] interface the mapper
//! \param[in] addr 16-bit address to read
//! \param[out] mappedAddr the translated address
typedef bool (*map_cpu_write_fn)(void *interface, uint16_t addr, uint32_t *mappedAddr);

//! \brief PPU read intercept
//! \param[in,out] interface the mapper
//! \param[in] addr 16-bit address to read
//! \param[out] mappedAddr the translated address
typedef bool (*map_ppu_read_fn)(void *interface, uint16_t addr, uint32_t *mappedAddr);

//! \brief PPU write intercept
//! \param[in,out] interface the mapper
//! \param[in] addr 16-bit address to read
//! \param[out] mappedAddr the translated address
typedef bool (*map_ppu_write_fn)(void *interface, uint16_t addr, uint32_t *mappedAddr);
