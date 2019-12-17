/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: bus.h
  Created: 2019-10-16
  Updated: 2019-12-06
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file bus.h
#ifndef BUS_VERSION
#define BUS_VERSION "0.1.0"

#include <stdint.h>
#include <stdbool.h>

struct apu;
struct bus;
struct cart;
struct cpu;
struct ppu;

struct controller {
        uint8_t input;
};

struct bus *
BusInit(struct apu *apu, struct cpu *cpu, struct ppu *ppu);

void
BusDeinit(struct bus *bus);

void
BusWrite(struct bus *bus, uint16_t addr, uint8_t data);

uint8_t
BusRead(struct bus *bus, uint16_t addr, bool readOnly);

void
BusReset(struct bus *bus);

//! \brief Increment the system by the fastest clock tick
//!
//! The running frequency is determined by the fastest clock in the system - in
//! this case, the PPU.  Every other clock is some fraction slower than the PPU,
//! so we compute a modulus every cycle to determine when to increment other
//! clocks.
//!
//! \param[in,out] bus
void
BusTick(struct bus *bus);

void
BusAttachCart(struct bus *bus, struct cart *cart);

struct controller *
BusGetControllers(struct bus *bus);

#endif // BUS_VERSION
