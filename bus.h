/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: bus.h
  Created: 2019-10-16
  Updated: 2019-10-17
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
#include <stdint.h>
//! \file bus.h

#ifndef BUS_VERSION
#define BUS_VERSION "0.1.0"

struct cpu;
struct bus;

struct bus *
BusInit(struct cpu *cpu);

void
BusDeinit(struct bus *bus);

void
BusWrite(struct bus *bus, uint16_t addr, uint8_t data);

uint8_t
BusRead(struct bus *bus, uint16_t addr);

#endif // BUS_VERSION
