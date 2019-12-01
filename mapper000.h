/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: mapper000.h
  Created: 2019-11-04
  Updated: 2019-11-27
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file mapper000.h
#include <stdint.h>
#include <stdbool.h>

struct mapper000;

void *
Mapper000_Init(uint8_t prgBanks, uint8_t chrBanks);

void
Mapper000_Deinit(void *mapper);

void
Mapper000_Reset(void *mapper);

//! \brief Map cpu read address to expanded address
//!
//! If PRGRPM is 16KB:
//!   Cpu Address Bus           PRG ROM
//!   ---------------           ---------
//!   0x8000 -> 0xBFFF: Map     0x0000 -> 0x3FFF
//!   0xC000 -> 0xFFFF: Mirror  0x0000 -> 0x3FFF
//!
//! If PRGRPM is 32KB:
//!   Cpu Address Bus           PRG ROM
//!   ---------------           ---------
//!   0x8000 -> 0xBFFF: Map     0x0000 -> 0x7FFF
//!
//! \see Mapper000_MapCpuWrite
//!
//! \param[in,out] mapper
//! \param[in] addr Address to be mapped
//! \param[out] mappedAddr Mapped address
//! \return true if address has been mapped
bool
Mapper000_MapCpuRead(void *mapper, uint16_t addr, uint32_t *mappedAddr);

//! \brief Map cpu write address to expanded address
//!
//! If PRGRPM is 16KB:
//!   Cpu Address Bus           PRG ROM
//!   ---------------           ---------
//!   0x8000 -> 0xBFFF: Map     0x0000 -> 0x3FFF
//!   0xC000 -> 0xFFFF: Mirror  0x0000 -> 0x3FFF
//!
//! If PRGRPM is 32KB:
//!   Cpu Address Bus           PRG ROM
//!   ---------------           ---------
//!   0x8000 -> 0xBFFF: Map     0x0000 -> 0x7FFF
//!
//! \see Mapper000_MapCpuRead
//!
//! \param[in,out] mapper
//! \param[in] addr Address to be mapped
//! \param[out] mappedAddr Mapped address
//! \return true if address has been mapped
bool
Mapper000_MapCpuWrite(void *mapper, uint16_t addr, uint32_t *mappedAddr);

//! \brief Map ppu read address to expanded address
//!
//! \see Mapper000_MapPpuWrite
//!
//! \param[in,out] mapper
//! \param[in] addr Address to be mapped
//! \param[out] mappedAddr Mapped address
//! \return true if address has been mapped
bool
Mapper000_MapPpuRead(void *mapper, uint16_t addr, uint32_t *mappedAddr);

//! \brief Map ppu write address to expanded address
//!
//! \see Mapper000_MapPpuRead
//!
//! \param[in,out] mapper
//! \param[in] addr Address to be mapped
//! \param[out] mappedAddr Mapped address
//! \return true if address has been mapped
bool
Mapper000_MapPpuWrite(void *mapper, uint16_t addr, uint32_t *mappedAddr);
