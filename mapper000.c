/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: mapper000.c
  Created: 2019-11-04
  Updated: 2019-11-04
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
#include <stdlib.h> // malloc, free

#include "mapper000.h"
//! \file mapper000.c

struct mapper000 {
        uint8_t prg_banks;
        uint8_t chr_banks;
};

void *Mapper000_Init(uint8_t prg_banks, uint8_t chr_banks) {
        struct mapper000 *mapper = (struct mapper000 *)malloc(sizeof(struct mapper000));
        if (NULL == mapper) {
                return NULL;
        }

        mapper->prg_banks = prg_banks;
        mapper->chr_banks = chr_banks;

        return (void *)mapper;
}

void Mapper000_Deinit(void *interface) {
        if (NULL == interface) {
                return;
        }

        free(interface);
        interface = NULL;
}

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
//! \param[out] mapped_addr Mapped address
//! \return true if address has been mapped
bool Mapper000_MapCpuRead(void *interface, uint16_t addr, uint32_t *mapped_addr) {
        struct mapper000 *mapper = (struct mapper000 *)interface;
        uint8_t mask = (mapper->prg_banks > 1) ? 0x7FFF : 0x3FFF;

        if (addr >= 0x8000 && addr <= 0xFFFF) {
                *mapped_addr = addr & mask;
                return true;
        }

        return false;
}

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
//! \param[out] mapped_addr Mapped address
//! \return true if address has been mapped
bool Mapper000_MapCpuWrite(void *interface, uint16_t addr, uint32_t *mapped_addr) {
        struct mapper000 *mapper = (struct mapper000 *)interface;
        uint8_t mask = (mapper->prg_banks > 1) ? 0x7FFF : 0x3FFF;

        if (addr >= 0x8000 && addr <= 0xFFFF) {
                *mapped_addr = addr & mask;
                return true;
        }

        return false;
}

//! \brief Map ppu read address to expanded address
//!
//! \see Mapper000_MapPpuWrite
//!
//! \param[in,out] mapper
//! \param[in] addr Address to be mapped
//! \param[out] mapped_addr Mapped address
//! \return true if address has been mapped
bool Mapper000_MapPpuRead(void *interface, uint16_t addr, uint32_t *mapped_addr) {
        if (addr >= 0x0000 && addr <= 0x1FFF) {
                *mapped_addr = addr;
                return true;
        }

        return false;
}

//! \brief Map ppu write address to expanded address
//!
//! \see Mapper000_MapPpuRead
//!
//! \param[in,out] mapper
//! \param[in] addr Address to be mapped
//! \param[out] mapped_addr Mapped address
//! \return true if address has been mapped
bool Mapper000_MapPpuWrite(void *interface, uint16_t addr, uint32_t *mapped_addr) {
        struct mapper000 *mapper = (struct mapper000 *)interface;
        if (addr >= 0x0000 && addr <= 0x1FFF) {
                if (0 == mapper->chr_banks) {
                        // Treat as RAM.
                        *mapped_addr = addr;
                        return true;
                }
        }

        return false;
}
