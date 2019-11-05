/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: bus.c
  Created: 2019-10-16
  Updated: 2019-11-05
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
#include "ppu.h"
#include "cart.h"
//! \file bus.c

struct bus {
        struct cpu *cpu;
        struct ppu *ppu;
        struct cart *cart;
        uint8_t *cpu_ram; // Dummy RAM for prototyping
        uint32_t tick_count;
};

struct bus *BusInit(struct cpu *cpu) {
        struct bus *bus = (struct bus *)malloc(sizeof(struct bus));

        bus->cpu_ram = (uint8_t *)malloc(64 * 1024);
        for (int i = 0; i < 64 * 1024; i++) {
                bus->cpu_ram[i] = 0x00;
        }

        bus->cpu = cpu;
        return bus;
}

void BusDeinit(struct bus *bus) {
        if (NULL == bus) {
                return;
        }

        if (NULL != bus->cpu_ram) {
                free(bus->cpu_ram);
        }

        free(bus);
}

void BusWrite(struct bus *bus, uint16_t addr, uint8_t data) {
        if (addr >= 0x0000 && addr <= 0xFFFF) {
                bus->cpu_ram[addr] = data;
        }
}

uint8_t BusRead(struct bus *bus, uint16_t addr) {
        if (addr >= 0x0000 && addr <= 0xFFFF) {
                return bus->cpu_ram[addr];
        }

        return 0x00;
}

void BusAttachCart(struct bus *bus, struct cart *cart) {
        bus->cart = cart;
        PpuAttachCart(bus->ppu, cart);
}

void BusReset(struct bus *bus) {
        CpuReset(bus->cpu);
        bus->tick_count = 0;
}

//! \brief Increment the system by the fastest clock tick
//!
//! The running frequency is determined by the fastest clock in the system - in
//! this case, the PPU.  Every other clock is some fraction slower than the PPU,
//! so we compute a modulus every cycle to determine when to increment other
//! clocks.
//!
//! \param[in,out] bus
void BusTick(struct bus *bus) {
        PpuTick(bus->ppu);

        if (0 == (bus->tick_count % 3)) {
                CpuTick(bus->cpu);
        }

        bus->tick_count++;
}
