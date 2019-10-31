/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: cpu.h
  Created: 2019-10-16
  Updated: 2019-10-17
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/

//! \file cpu.h

#ifndef CPU_VERSION
#define CPU_VERSION "0.1.0"

struct cpu;
struct bus;

struct cpu *
CpuInit();

void
CpuDeinit(struct cpu *cpu);

void
CpuConnectBus(struct cpu *cpu, struct bus *bus);

void Clock();
void Reset();
void Irq();
void Nmi();

#endif // CPU_VERSION
