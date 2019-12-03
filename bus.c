/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: bus.c
  Created: 2019-10-16
  Updated: 2019-11-30
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file bus.c
#include <stdlib.h> // calloc, free

#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "cart.h"
#include "util.h"

struct bus {
        struct cpu *cpu;
        struct ppu *ppu;
        struct cart *cart;
        uint8_t *cpuRam; // Dummy RAM for prototyping
        uint32_t tickCount;
};

struct bus *BusInit(struct cpu *cpu, struct ppu *ppu) {
        struct bus *bus = (struct bus *)calloc(1, sizeof(struct bus));
        if (NULL == bus)
                return NULL;

        bus->cpuRam = (uint8_t *)calloc(KB_AS_B(2), sizeof(uint8_t));
        if (NULL == bus->cpuRam) {
                BusDeinit(bus);
                return NULL;
        }

        bus->cpu = cpu;
        bus->ppu = ppu;

        return bus;
}

void BusDeinit(struct bus *bus) {
        if (NULL == bus) {
                return;
        }

        if (NULL != bus->cpuRam) {
                free(bus->cpuRam);
        }

        free(bus);
}

void BusWrite(struct bus *bus, uint16_t addr, uint8_t data) {
        if (CartCpuWrite(bus->cart, addr, data)) {

        } else if (addr >= 0x0000 && addr <= 0x1FFF) {
                // System RAM address range, mirrored every 2048.
                bus->cpuRam[addr & 0x07FF] = data;
        } else if (addr >= 0x2000 && addr <= 0x3FFF) {
                PpuWriteViaCpu(bus->ppu, addr & 0x0007, data);
        }
}

uint8_t BusRead(struct bus *bus, uint16_t addr, bool readOnly) {
        uint8_t data = 0x00;

        if (CartCpuRead(bus->cart, addr, &data)) {
                // Cartridge address range
        } else if (addr >= 0x0000 && addr <= 0x1FFF) {
                // System RAM address range, mirrored every 2048.
                data = bus->cpuRam[addr & 0x07FF];
        } else if (addr >= 0x2000 && addr <= 0x3FFF) {
                // PPU address range, mirrored every 8.
                data = PpuReadViaCpu(bus->ppu, addr & 0x0007, readOnly);
        }

        return data;
}

void BusAttachCart(struct bus *bus, struct cart *cart) {
        bus->cart = cart;
        PpuAttachCart(bus->ppu, cart);
}

void BusReset(struct bus *bus) {
        CartReset(bus->cart);
        CpuReset(bus->cpu);
        PpuReset(bus->ppu);
        bus->tickCount = 0;
}

void BusTick(struct bus *bus) {
        PpuTick(bus->ppu);

        if (0 == (bus->tickCount % 3)) {
                CpuTick(bus->cpu);
        }

        if (PpuGetNmi(bus->ppu)) {
                PpuSetNmi(bus->ppu, false);
                CpuNmi(bus->cpu);
        }

        bus->tickCount++;
}
