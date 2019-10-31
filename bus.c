/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: bus.c
  Created: 2019-10-16
  Updated: 2019-10-31
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
#include <stdlib.h> // malloc, free

#include "bus.h"
#include "cpu.h"
//! \file bus.c

struct bus {
        struct cpu *cpu;
        uint8_t *ram; // Dummy RAM for prototyping
};

struct bus *BusInit(struct cpu *cpu) {
        struct bus *bus = (struct bus *)malloc(sizeof(struct bus));

        bus->ram = (uint8_t *)malloc(64 * 1024);
        for (int i = 0; i < 64 * 1024; i++) {
                bus->ram[i] = 0x00;
        }

        bus->cpu = cpu;
        return bus;
}

void BusDeinit(struct bus *bus) {
        if (NULL == bus) {
                return;
        }

        if (NULL != bus->ram) {
                free(bus->ram);
        }

        free(bus);
}

void BusWrite(struct bus *bus, uint16_t addr, uint8_t data) {
        if (addr >= 0x0000 && addr <= 0xFFFF) {
                bus->ram[addr] = data;
        }
}

uint8_t BusRead(struct bus *bus, uint16_t addr) {
        if (addr >= 0x0000 && addr <= 0xFFFF) {
                return bus->ram[addr];
        }

        return 0x00;
}
