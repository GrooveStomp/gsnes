/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: cpu.h
  Created: 2019-10-16
  Updated: 2019-11-21
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file cpu.h
#include <stdint.h>

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

void
CpuReset(struct cpu *cpu);

void
CpuTick(struct cpu *cpu);

int
CpuIsComplete(struct cpu *cpu);

void
CpuNmi(struct cpu *cpu);

//-- Debug ---------------------------------------------------------------------

char **
CpuDebugStateInit(struct cpu *cpu);

void
CpuDebugStateDeinit(char **debug);

struct debug_instruction {
        uint16_t address;
        char *text;
        int text_length;
};

struct disassembly {
        struct debug_instruction **map;
        int count;
};

void
DebugInstructionDeinit(struct debug_instruction *inst);

struct disassembly *
DisassemblyInit(struct cpu *cpu, uint16_t start, uint16_t stop);

void
DisassemblyDeinit(struct disassembly *disassembly);

int
DisassemblyFindPc(struct disassembly *disassembly, struct cpu *cpu);

#endif // CPU_VERSION
