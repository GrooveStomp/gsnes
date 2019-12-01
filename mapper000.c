/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: mapper000.c
  Created: 2019-11-04
  Updated: 2019-11-27
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file mapper000.c
#include <stdlib.h> // malloc, free

#include "mapper000.h"

struct mapper000 {
        uint8_t prgBanks;
        uint8_t chrBanks;
};

void *Mapper000_Init(uint8_t prgBanks, uint8_t chrBanks) {
        struct mapper000 *mapper = (struct mapper000 *)malloc(sizeof(struct mapper000));
        if (NULL == mapper) {
                return NULL;
        }

        mapper->prgBanks = prgBanks;
        mapper->chrBanks = chrBanks;

        return (void *)mapper;
}

void Mapper000_Deinit(void *interface) {
        if (NULL == interface)
                return;

        free(interface);
}

void Mapper000_Reset(void *mapper) {
}

bool Mapper000_MapCpuRead(void *interface, uint16_t addr, uint32_t *mappedAddr) {
        struct mapper000 *mapper = (struct mapper000 *)interface;
        uint8_t mask = (mapper->prgBanks > 1) ? 0x7FFF : 0x3FFF;

        if (addr >= 0x8000 && addr <= 0xFFFF) {
                *mappedAddr = addr & mask;
                return true;
        }

        return false;
}

bool Mapper000_MapCpuWrite(void *interface, uint16_t addr, uint32_t *mappedAddr) {
        struct mapper000 *mapper = (struct mapper000 *)interface;
        uint8_t mask = (mapper->prgBanks > 1) ? 0x7FFF : 0x3FFF;

        if (addr >= 0x8000 && addr <= 0xFFFF) {
                *mappedAddr = addr & mask;
                return true;
        }

        return false;
}

bool Mapper000_MapPpuRead(void *interface, uint16_t addr, uint32_t *mappedAddr) {
        if (addr >= 0x0000 && addr <= 0x1FFF) {
                *mappedAddr = addr;
                return true;
        }

        return false;
}

bool Mapper000_MapPpuWrite(void *interface, uint16_t addr, uint32_t *mappedAddr) {
        struct mapper000 *mapper = (struct mapper000 *)interface;
        if (addr >= 0x0000 && addr <= 0x1FFF) {
                if (0 == mapper->chrBanks) {
                        // Treat as RAM.
                        *mappedAddr = addr;
                        return true;
                }
        }

        return false;
}
