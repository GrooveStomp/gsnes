
/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: cpu.c
  Created: 2019-10-16
  Updated: 2019-11-05
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: olcNES Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
#include <stdbool.h> // bool
#include <stdio.h> // snprintf
#include <stdlib.h> // malloc, free
#include <string.h> // strncpy

#include "cpu.h"
#include "bus.h"

//! \file cpu.c

// Addressing Modes
uint8_t ABS(struct cpu *cpu);
uint8_t ABX(struct cpu *cpu);
uint8_t ABY(struct cpu *cpu);
uint8_t IMM(struct cpu *cpu);
uint8_t IMP(struct cpu *cpu);
uint8_t IND(struct cpu *cpu);
uint8_t IZX(struct cpu *cpu);
uint8_t IZY(struct cpu *cpu);
uint8_t REL(struct cpu *cpu);
uint8_t ZP0(struct cpu *cpu);
uint8_t ZPX(struct cpu *cpu);
uint8_t ZPY(struct cpu *cpu);

// Opcodes
uint8_t XXX(struct cpu *cpu); //<! Illegal Opcode

uint8_t ADC(struct cpu *cpu); uint8_t AND(struct cpu *cpu); uint8_t ASL(struct cpu *cpu); uint8_t BCC(struct cpu *cpu);
uint8_t BCS(struct cpu *cpu); uint8_t BEQ(struct cpu *cpu); uint8_t BIT(struct cpu *cpu); uint8_t BMI(struct cpu *cpu);
uint8_t BNE(struct cpu *cpu); uint8_t BPL(struct cpu *cpu); uint8_t BRK(struct cpu *cpu); uint8_t BVC(struct cpu *cpu);
uint8_t BVS(struct cpu *cpu); uint8_t CLC(struct cpu *cpu); uint8_t CLD(struct cpu *cpu); uint8_t CLI(struct cpu *cpu);
uint8_t CLV(struct cpu *cpu); uint8_t CMP(struct cpu *cpu); uint8_t CPX(struct cpu *cpu); uint8_t CPY(struct cpu *cpu);
uint8_t DEC(struct cpu *cpu); uint8_t DEX(struct cpu *cpu); uint8_t DEY(struct cpu *cpu); uint8_t EOR(struct cpu *cpu);
uint8_t INC(struct cpu *cpu); uint8_t INX(struct cpu *cpu); uint8_t INY(struct cpu *cpu); uint8_t JMP(struct cpu *cpu);
uint8_t JSR(struct cpu *cpu); uint8_t LDA(struct cpu *cpu); uint8_t LDX(struct cpu *cpu); uint8_t LDY(struct cpu *cpu);
uint8_t LSR(struct cpu *cpu); uint8_t NOP(struct cpu *cpu); uint8_t ORA(struct cpu *cpu); uint8_t PHA(struct cpu *cpu);
uint8_t PHP(struct cpu *cpu); uint8_t PLA(struct cpu *cpu); uint8_t PLP(struct cpu *cpu); uint8_t ROL(struct cpu *cpu);
uint8_t ROR(struct cpu *cpu); uint8_t RTI(struct cpu *cpu); uint8_t RTS(struct cpu *cpu); uint8_t SBC(struct cpu *cpu);
uint8_t SEC(struct cpu *cpu); uint8_t SED(struct cpu *cpu); uint8_t SEI(struct cpu *cpu); uint8_t STA(struct cpu *cpu);
uint8_t STX(struct cpu *cpu); uint8_t STY(struct cpu *cpu); uint8_t TAX(struct cpu *cpu); uint8_t TAY(struct cpu *cpu);
uint8_t TSX(struct cpu *cpu); uint8_t TXA(struct cpu *cpu); uint8_t TXS(struct cpu *cpu); uint8_t TYA(struct cpu *cpu);

struct instruction {
        char *name;
        uint8_t (*operate)(struct cpu *cpu);
        uint8_t (*address)(struct cpu *cpu);
        uint8_t cycles;
};

static struct instruction instruction_map[] = {
        { "BRK", BRK, IMM, 7 },{ "ORA", ORA, IZX, 6 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 3 },{ "ORA", ORA, ZP0, 3 },{ "ASL", ASL, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "PHP", PHP, IMP, 3 },{ "ORA", ORA, IMM, 2 },{ "ASL", ASL, IMP, 2 },{ "???", XXX, IMP, 2 },{ "???", NOP, IMP, 4 },{ "ORA", ORA, ABS, 4 },{ "ASL", ASL, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BPL", BPL, REL, 2 },{ "ORA", ORA, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "ORA", ORA, ZPX, 4 },{ "ASL", ASL, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "CLC", CLC, IMP, 2 },{ "ORA", ORA, ABY, 4 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "ORA", ORA, ABX, 4 },{ "ASL", ASL, ABX, 7 },{ "???", XXX, IMP, 7 },
        { "JSR", JSR, ABS, 6 },{ "AND", AND, IZX, 6 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "BIT", BIT, ZP0, 3 },{ "AND", AND, ZP0, 3 },{ "ROL", ROL, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "PLP", PLP, IMP, 4 },{ "AND", AND, IMM, 2 },{ "ROL", ROL, IMP, 2 },{ "???", XXX, IMP, 2 },{ "BIT", BIT, ABS, 4 },{ "AND", AND, ABS, 4 },{ "ROL", ROL, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BMI", BMI, REL, 2 },{ "AND", AND, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "AND", AND, ZPX, 4 },{ "ROL", ROL, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "SEC", SEC, IMP, 2 },{ "AND", AND, ABY, 4 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "AND", AND, ABX, 4 },{ "ROL", ROL, ABX, 7 },{ "???", XXX, IMP, 7 },
        { "RTI", RTI, IMP, 6 },{ "EOR", EOR, IZX, 6 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 3 },{ "EOR", EOR, ZP0, 3 },{ "LSR", LSR, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "PHA", PHA, IMP, 3 },{ "EOR", EOR, IMM, 2 },{ "LSR", LSR, IMP, 2 },{ "???", XXX, IMP, 2 },{ "JMP", JMP, ABS, 3 },{ "EOR", EOR, ABS, 4 },{ "LSR", LSR, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BVC", BVC, REL, 2 },{ "EOR", EOR, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "EOR", EOR, ZPX, 4 },{ "LSR", LSR, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "CLI", CLI, IMP, 2 },{ "EOR", EOR, ABY, 4 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "EOR", EOR, ABX, 4 },{ "LSR", LSR, ABX, 7 },{ "???", XXX, IMP, 7 },
        { "RTS", RTS, IMP, 6 },{ "ADC", ADC, IZX, 6 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 3 },{ "ADC", ADC, ZP0, 3 },{ "ROR", ROR, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "PLA", PLA, IMP, 4 },{ "ADC", ADC, IMM, 2 },{ "ROR", ROR, IMP, 2 },{ "???", XXX, IMP, 2 },{ "JMP", JMP, IND, 5 },{ "ADC", ADC, ABS, 4 },{ "ROR", ROR, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BVS", BVS, REL, 2 },{ "ADC", ADC, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "ADC", ADC, ZPX, 4 },{ "ROR", ROR, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "SEI", SEI, IMP, 2 },{ "ADC", ADC, ABY, 4 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "ADC", ADC, ABX, 4 },{ "ROR", ROR, ABX, 7 },{ "???", XXX, IMP, 7 },
        { "???", NOP, IMP, 2 },{ "STA", STA, IZX, 6 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 6 },{ "STY", STY, ZP0, 3 },{ "STA", STA, ZP0, 3 },{ "STX", STX, ZP0, 3 },{ "???", XXX, IMP, 3 },{ "DEY", DEY, IMP, 2 },{ "???", NOP, IMP, 2 },{ "TXA", TXA, IMP, 2 },{ "???", XXX, IMP, 2 },{ "STY", STY, ABS, 4 },{ "STA", STA, ABS, 4 },{ "STX", STX, ABS, 4 },{ "???", XXX, IMP, 4 },
        { "BCC", BCC, REL, 2 },{ "STA", STA, IZY, 6 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 6 },{ "STY", STY, ZPX, 4 },{ "STA", STA, ZPX, 4 },{ "STX", STX, ZPY, 4 },{ "???", XXX, IMP, 4 },{ "TYA", TYA, IMP, 2 },{ "STA", STA, ABY, 5 },{ "TXS", TXS, IMP, 2 },{ "???", XXX, IMP, 5 },{ "???", NOP, IMP, 5 },{ "STA", STA, ABX, 5 },{ "???", XXX, IMP, 5 },{ "???", XXX, IMP, 5 },
        { "LDY", LDY, IMM, 2 },{ "LDA", LDA, IZX, 6 },{ "LDX", LDX, IMM, 2 },{ "???", XXX, IMP, 6 },{ "LDY", LDY, ZP0, 3 },{ "LDA", LDA, ZP0, 3 },{ "LDX", LDX, ZP0, 3 },{ "???", XXX, IMP, 3 },{ "TAY", TAY, IMP, 2 },{ "LDA", LDA, IMM, 2 },{ "TAX", TAX, IMP, 2 },{ "???", XXX, IMP, 2 },{ "LDY", LDY, ABS, 4 },{ "LDA", LDA, ABS, 4 },{ "LDX", LDX, ABS, 4 },{ "???", XXX, IMP, 4 },
        { "BCS", BCS, REL, 2 },{ "LDA", LDA, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 5 },{ "LDY", LDY, ZPX, 4 },{ "LDA", LDA, ZPX, 4 },{ "LDX", LDX, ZPY, 4 },{ "???", XXX, IMP, 4 },{ "CLV", CLV, IMP, 2 },{ "LDA", LDA, ABY, 4 },{ "TSX", TSX, IMP, 2 },{ "???", XXX, IMP, 4 },{ "LDY", LDY, ABX, 4 },{ "LDA", LDA, ABX, 4 },{ "LDX", LDX, ABY, 4 },{ "???", XXX, IMP, 4 },
        { "CPY", CPY, IMM, 2 },{ "CMP", CMP, IZX, 6 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 8 },{ "CPY", CPY, ZP0, 3 },{ "CMP", CMP, ZP0, 3 },{ "DEC", DEC, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "INY", INY, IMP, 2 },{ "CMP", CMP, IMM, 2 },{ "DEX", DEX, IMP, 2 },{ "???", XXX, IMP, 2 },{ "CPY", CPY, ABS, 4 },{ "CMP", CMP, ABS, 4 },{ "DEC", DEC, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BNE", BNE, REL, 2 },{ "CMP", CMP, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "CMP", CMP, ZPX, 4 },{ "DEC", DEC, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "CLD", CLD, IMP, 2 },{ "CMP", CMP, ABY, 4 },{ "NOP", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "CMP", CMP, ABX, 4 },{ "DEC", DEC, ABX, 7 },{ "???", XXX, IMP, 7 },
        { "CPX", CPX, IMM, 2 },{ "SBC", SBC, IZX, 6 },{ "???", NOP, IMP, 2 },{ "???", XXX, IMP, 8 },{ "CPX", CPX, ZP0, 3 },{ "SBC", SBC, ZP0, 3 },{ "INC", INC, ZP0, 5 },{ "???", XXX, IMP, 5 },{ "INX", INX, IMP, 2 },{ "SBC", SBC, IMM, 2 },{ "NOP", NOP, IMP, 2 },{ "???", SBC, IMP, 2 },{ "CPX", CPX, ABS, 4 },{ "SBC", SBC, ABS, 4 },{ "INC", INC, ABS, 6 },{ "???", XXX, IMP, 6 },
        { "BEQ", BEQ, REL, 2 },{ "SBC", SBC, IZY, 5 },{ "???", XXX, IMP, 2 },{ "???", XXX, IMP, 8 },{ "???", NOP, IMP, 4 },{ "SBC", SBC, ZPX, 4 },{ "INC", INC, ZPX, 6 },{ "???", XXX, IMP, 6 },{ "SED", SED, IMP, 2 },{ "SBC", SBC, ABY, 4 },{ "NOP", NOP, IMP, 2 },{ "???", XXX, IMP, 7 },{ "???", NOP, IMP, 4 },{ "SBC", SBC, ABX, 4 },{ "INC", INC, ABX, 7 },{ "???", XXX, IMP, 7 },
};

struct cpu {
        struct bus *bus;
        uint8_t a; //!< Accumulator Register
        uint8_t x;
        uint8_t y;
        uint8_t sp; //!< Stack Pointer
        uint16_t pc; //!< Program Counter
        uint8_t status; //!< Status Register

        uint8_t fetched; //!< Any data fetched for the current instruction
        uint16_t addr_abs;
        uint16_t addr_rel;
        uint8_t opcode; //!< Opcode for the currently executing instruction
        uint8_t cycles; //!< How many cycles the current instruction takes
        uint32_t tick_count;
};

enum status_flags {
        C = (1 << 0), //!< Carry Bit
        Z = (1 << 1), //!< Zero Bit
        I = (1 << 2), //!< Disable Interrupts
        D = (1 << 3), //!< Decimal Mode
        B = (1 << 4), //!< Break
        U = (1 << 5), //!< Unused
        V = (1 << 6), //!< Overflow
        N = (1 << 7), //!< Negative
};

struct cpu *CpuInit() {
        struct cpu *cpu = (struct cpu *)malloc(sizeof(struct cpu));
        cpu->bus = NULL;
        return cpu;

        cpu->a = 0x00;
        cpu->x = 0x00;
        cpu->y = 0x00;
        cpu->sp = 0x00;
        cpu->pc = 0x0000;
        cpu->status = 0x00;

        cpu->addr_abs = 0x0000;
        cpu->addr_rel = 0x00;
        cpu->opcode = 0x00;
        cpu->cycles = 0;
        cpu->tick_count = 0;
}

void CpuDeinit(struct cpu *cpu) {
        if (NULL != cpu) {
                free(cpu);
        }
}

void CpuConnectBus(struct cpu *cpu, struct bus *bus) {
        cpu->bus = bus;
}

static uint8_t GetFlag(struct cpu *cpu, enum status_flags f) {
        return ((cpu->status & f) > 0) ? 1 : 0;
}

static void SetFlag(struct cpu *cpu, enum status_flags f, bool v) {
        if (v)
                cpu->status |= f;
        else
                cpu->status &= ~f;
}

static uint8_t Fetch(struct cpu* cpu) {
        if (!(instruction_map[cpu->opcode].address == IMP)) {
                cpu->fetched = BusRead(cpu->bus, cpu->addr_abs);
        }

        return cpu->fetched;
}

//! \brief Simulate clock ticks
void CpuTick(struct cpu *cpu) {
        if (0 == cpu->cycles) {
                // Read the next byte to determine which opcode we are using.
                cpu->opcode = BusRead(cpu->bus, cpu->pc);
                cpu->pc++;

                // Now use the instruction map to get the instruction our opcode is implementing.
                struct instruction *instruction = &instruction_map[cpu->opcode];

                cpu->cycles = instruction->cycles; // Get starting number of cycles
                uint8_t need_more_cycles_1 = instruction->address(cpu);
                uint8_t need_more_cycles_2 = instruction->operate(cpu);

                // If both the address and operate functions indicate that an
                // additional cycle was required, then increase the number of
                // cycles by 1.
                cpu->cycles += (need_more_cycles_1 & need_more_cycles_2);
        }

        cpu->tick_count++;
        cpu->cycles--;
}

void CpuReset(struct cpu *cpu) {
        cpu->a = 0;
        cpu->x = 0;
        cpu->y = 0;
        cpu->sp = 0xFD;
        cpu->status = 0x00 | U;

        cpu->addr_abs = 0xFFFC;
        uint16_t lo = BusRead(cpu->bus, cpu->addr_abs + 0);
        uint16_t hi = BusRead(cpu->bus, cpu->addr_abs + 1);

        cpu->pc = (hi << 8) | lo;

        cpu->addr_rel = 0x0000;
        cpu->addr_abs = 0x0000;
        cpu->fetched = 0x00;

        cpu->cycles = 8;
}

void Irq(struct cpu *cpu) {
        if (GetFlag(cpu, I) == 0) {
                BusWrite(cpu->bus, 0x0100 + cpu->sp, (cpu->pc >> 8) & 0x00FF);
                cpu->sp--;
                BusWrite(cpu->bus, 0x0100 + cpu->sp, cpu->pc & 0x00FF);
                cpu->sp--;

                SetFlag(cpu, B, 0);
                SetFlag(cpu, U, 1);
                SetFlag(cpu, I, 1);
                BusWrite(cpu->bus, 0x0100 + cpu->sp, cpu->status);
                cpu->sp--;

                cpu->addr_abs = 0xFFFE;
                uint16_t lo = BusRead(cpu->bus, cpu->addr_abs + 0);
                uint16_t hi = BusRead(cpu->bus, cpu->addr_abs + 1);
                cpu->pc = (hi << 8) | lo;

                cpu->cycles = 7;
        }
}

void Nmi(struct cpu *cpu) {
        BusWrite(cpu->bus, 0x0100 + cpu->sp, (cpu->pc >> 8) & 0x00FF);
        cpu->sp--;
        BusWrite(cpu->bus, 0x0100 + cpu->sp, cpu->pc & 0x00FF);
        cpu->sp--;

        SetFlag(cpu, B, 0);
        SetFlag(cpu, U, 1);
        SetFlag(cpu, I, 1);
        BusWrite(cpu->bus, 0x0100 + cpu->sp, cpu->status);
        cpu->sp--;

        cpu->addr_abs = 0xFFFA;
        uint16_t lo = BusRead(cpu->bus, cpu->addr_abs + 0);
        uint16_t hi = BusRead(cpu->bus, cpu->addr_abs + 1);
        cpu->pc = (hi << 8) | lo;

        cpu->cycles = 8;
}


//-- Addressing Modes ----------------------------------------------------------


//! \brief Add with Carry
//!
//! \param[in,out] cpu
//! \return 1 This addressing mode will incur another cycle if the instruction
//! demands it
uint8_t ADC(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);

        // Add is performed in the 16-bit domain for emulation to capture the
        // carry bit; which will exist in bit 8 of the 16-bit world.
        uint16_t tmp = (uint16_t)cpu->a + (uint16_t)fetched + (uint16_t)GetFlag(cpu, C);

        SetFlag(cpu, C, tmp > 255);
        SetFlag(cpu, Z, (tmp & 0x00FF) == 0);

        // The signed Overflow flag
        SetFlag(cpu, V, (~((uint16_t)cpu->a ^ (uint16_t)fetched) & ((uint16_t)cpu->a ^ (uint16_t)tmp)) & 0x0000);

        SetFlag(cpu, N, tmp & 0x00);

        cpu->a = tmp & 0x00FF;

        return 1;
}

//! \brief Subtraction with Borrow
//!
//! \param[in,out] cpu
//! \return int 1 if this instruction _can_ take another clock cycle, else 0
uint8_t SBC(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);

        // Invert the bottom 8 bits with bitwise xor.
        uint16_t value = ((uint16_t)fetched) ^ 0x00FF;

        // Subtraction is almost exactly the same as addition now, except for signed overflow.

        uint16_t tmp = (uint16_t)cpu->a + (uint16_t)value + (uint16_t)GetFlag(cpu, C);

        SetFlag(cpu, C, tmp > 255);
        SetFlag(cpu, Z, (tmp & 0x00FF) == 0);

        // The signed Overflow flag is different from addition.
        SetFlag(cpu, V, (tmp ^ (uint16_t)cpu->a) & (tmp ^ value) & 0x0000);

        SetFlag(cpu, N, tmp & 0x00);

        cpu->a = tmp & 0x00FF;

        return 1;
}

//! Absolute Addressing
//!
//! Read the absolute 16-bit address from the location pointed to by the program
//! counter.
//!
//! \param[in,out] cpu
//! \return 0 This addressing mode will take no additional cycles
uint8_t ABS(struct cpu *cpu) {
        uint16_t lo = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;
        uint16_t hi = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;

        cpu->addr_abs = (hi << 8) | lo;

        return 0;
}

//!
//! \param[in,out] cpu
//! \return int 1 if this addressing mode _can_ take another clock cycle, else 0
uint8_t ABX(struct cpu *cpu) {
        uint16_t lo = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;
        uint16_t hi = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;

        cpu->addr_abs = (hi << 8) | lo;
        cpu->addr_abs += cpu->x;

        // If the memory page has changed, then this addressing mode may take
        // another clock cycle to execute.
        if ((cpu->addr_abs & 0xFF00) != (hi << 8))
                return 1;
        else
                return 0;
}

//!
//! \param[in,out] cpu
//! \return int 1 if this addressing mode _can_ take another clock cycle, else 0
uint8_t ABY(struct cpu *cpu) {
        uint16_t lo = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;
        uint16_t hi = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;

        cpu->addr_abs = (hi << 8) | lo;
        cpu->addr_abs += cpu->y;

        // If the memory page has changed, then this addressing mode may take
        // another clock cycle to execute.
        if ((cpu->addr_abs & 0xFF00) != (hi << 8))
                return 1;
        else
                return 0;
}

//! \brief Immediate Mode Addressing
//!
//! The next byte pointed to by the PC identifies the data required for the
//! current instruction.
//!
//! \param[in,out] cpu
//! \return 0 This addressing mode will take no additional cycles
uint8_t IMM(struct cpu *cpu) {
        cpu->addr_abs = cpu->pc++;
        return 0;
}

//! \brief Implicit Addressing
//!
//! No explicit data as part of the instruction, but the cpu may use the
//! accumulator register.
//!
//! \param[in,out] cpu
//! \return 0 This addressing mode will take no additional cycles
uint8_t IMP(struct cpu *cpu) {
        cpu->fetched = cpu->a;
        return 0;
}

//! \brief Indirect Addressing (aka Pointer to memory)
//!
//! Read the 16-bit address pointed to by the program counter to get the 16-bit
//! address containing the data we need for this instruction.
//!
//! There is a bug in the original hardware:
//!
//! If the low byte of the supplied address is 0xFF, then to read the high byte
//! of the actual address we need to cross a page boundary.  Unfortunately, the
//! bug causes the page to wrap around to the same page, yielding an invalid
//! address.
//!
//! \param[in,out] cpu
//! \return 0 This addressing mode will take no additional cycles
uint8_t IND(struct cpu *cpu) {
        uint16_t ptr_lo = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;
        uint16_t ptr_hi = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;

        uint16_t ptr = (ptr_hi << 8) | ptr_lo;

        if (ptr_lo == 0x00FF) { // Simulate page boundary hardware bug
                cpu->addr_abs = (BusRead(cpu->bus, ptr & 0xFF00) << 8) | BusRead(cpu->bus, ptr + 0);
        } else { // Behave normally
                cpu->addr_abs = (BusRead(cpu->bus, ptr + 1) << 8) | BusRead(cpu->bus, ptr + 0);
        }

        return 0;
}

//! \brief Indirect Zero-Pate Addressing with X Register Offset
//!
//! Read the zeroth page address to be read from memory.  Now offset the address
//! to be read by the contents of the X register then read the resulting
//! address and store the result.
//!
//! \param[in,out] cpu
//! \return 0 This addressing mode will take no additional cycles
uint8_t IZX(struct cpu *cpu) {
        uint16_t t = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;

        uint16_t offset = t + (uint16_t)(cpu->x);
        uint16_t lo = BusRead(cpu->bus, offset & 0x00FF);
        uint16_t hi = BusRead(cpu->bus, (offset +  1) & 0x00FF);

        cpu->addr_abs = (hi << 8) | lo;

        return 0;
}

//! \brief Indirect Zero-Page Addressing with Y Register Offset
//!
//! This behaves differently from IZX.
//!
//! Read the zeroth page address to be read from memory.  Now read the value at
//! the address to be read; add the contents of the Y register to this. Store
//! the resulting address.
//!
//! NOTE: This can cause a memory page boundary, in which case it consumes an
//! extra CPU cycle.
//!
//! \param[in,out] cpu
//! \return int 1 if this addressing mode _can_ take another clock cycle, else 0
uint8_t IZY(struct cpu *cpu) {
        uint16_t t = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;

        uint16_t lo = BusRead(cpu->bus, t & 0x00FF);
        uint16_t hi = BusRead(cpu->bus, (t + 1) & 0x00FF);

        cpu->addr_abs = (hi << 8) | lo;
        cpu->addr_abs += cpu->y;

        if ((cpu->addr_abs & 0xFF00) != (hi << 8))
                return 1;
        else
                return 0;
}

//! \brief Relative Addressing
//!
//! This address mode is exclusive to branch instructions. The address must
//! reside within -128 to +127 of the branch instruction, i.e.  you cant
//! directly branch to any address in the addressable range.
//!
//! \param[in,out] cpu
//! \return 0 This addressing mode will take no additional cycles
uint8_t REL(struct cpu *cpu) {
        cpu->addr_rel = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;

        // REL involves signed values for jumps.
        // Signed values are represented in 2's completement notation.
        // To get the 2's complement of a number, negate its bits then add one.
        // Negative numbesr are represented with leading 1s, while positive
        // numbers have leading 0s.

        // Check if the leading bit of this byte is a 1, indicating a negative
        // number.
        if (cpu->addr_rel & 0x80) {
                // If addr_rel is negative, then sign-extend the address to the
                // full 16-bits so the addition works out.
                cpu->addr_rel |= 0xFF00;
        }

        return 0;
}

//! \brief Zero-Page Addressing
//!
//! Data for this instruction is in the zeroth page of memory, so the address is
//! an 8-bit pointer instead of a 16-bit pointer.
//!
//! \param[in,out] cpu
//! \return 0 This addressing mode will take no additional cycles
uint8_t ZP0(struct cpu *cpu) {
        cpu->addr_abs = BusRead(cpu->bus, cpu->pc);
        cpu->pc++;
        cpu->addr_abs &= 0x00FF;
        return 0;
}

//! \brief Zero-Page Addressing with X offset
//!
//! The X register specifies an offset for the Zero-Page address. Useful for
//! iterating through memory in the zeroth page.
//!
//! \param[in,out] cpu
//! \return 0 This addressing mode will take no additional cycles
uint8_t ZPX(struct cpu *cpu) {
        cpu->addr_abs = BusRead(cpu->bus, cpu->pc) + cpu->x;
        cpu->pc++;
        cpu->addr_abs &= 0x00FF;
        return 0;
}

//! \brief Zero-Page Addressing with Y offset
//!
//! The Y register specifies an offset for the Zero-Page address. Useful for
//! iterating through memory in the zeroth page.
//!
//! \param[in,out] cpu
//! \return 0 This addressing mode will take no additional cycles
uint8_t ZPY(struct cpu *cpu) {
        cpu->addr_abs = BusRead(cpu->bus, cpu->pc) + cpu->y;
        cpu->pc++;
        cpu->addr_abs &= 0x00FF;
        return 0;
}


//-- Instructions --------------------------------------------------------------


//! \brief Bitwise logical AND
//!
//! \param[in,out] cpu
//! \return 1 This instruction will incur another cycle if the addressing mode
//! demands it
uint8_t AND(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        cpu->a = cpu->a & fetched;
        SetFlag(cpu, Z, cpu->a == 0x00);
        SetFlag(cpu, N, cpu->a & 0x80);
        return 1;
}

//! \brief Arithmetic Shift Left
//!
//! \param[in,out] cpu
//! \return 1 This instruction will incur another cycle if the addressing mode
//! demands it
uint8_t ASL(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);

        uint16_t tmp = (uint16_t)fetched << 1;
        SetFlag(cpu, C, (tmp & 0xFF00) > 0);
        SetFlag(cpu, Z, (tmp & 0x00FF) == 0x00);
        SetFlag(cpu, N, tmp & 0x80);
        if (instruction_map[cpu->opcode].address == IMP) {
                cpu->a = tmp & 0x00FF;
        } else {
                BusWrite(cpu->bus, cpu->addr_abs, tmp & 0x00FF);
        }

        return 0;
}

//! \brief Branch if Carry Clear
//!
//! Set the program counter to the loaded address if the carry flag is not set.
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t BCC(struct cpu *cpu) {
        if (GetFlag(cpu, C) == 0) {
                // Branch instructions implicitly increment clock cycles.
                cpu->cycles++;
                cpu->addr_abs = cpu->pc + cpu->addr_rel;

                // If the memory page is different, then incur another clock
                // cycle.
                if ((cpu->addr_abs & 0xFF00) != (cpu->pc & 0xFF00)) {
                        cpu->cycles++;
                }

                // Set the program counter.
                cpu->pc = cpu->addr_abs;
        }

        return 0;
}

//! \brief Branch if Carry Set
//!
//! if (C == 1) pc = address
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t BCS(struct cpu *cpu) {
        if (GetFlag(cpu, C) == 1) {
                // Branch instructions implicitly increment clock cycles.
                cpu->cycles++;
                cpu->addr_abs = cpu->pc + cpu->addr_rel;

                // If the memory page is different, then incur another clock
                // cycle.
                if ((cpu->addr_abs & 0xFF00) != (cpu->pc & 0xFF00)) {
                        cpu->cycles++;
                }

                // Set the program counter.
                cpu->pc = cpu->addr_abs;
        }

        return 0;
}

//! \brief Branch if Equal
//!
//! if (Z == 1) pc = address
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t BEQ(struct cpu *cpu) {
        if (GetFlag(cpu, Z) == 1) {
                // Branch instructions implicitly increment clock cycles.
                cpu->cycles++;
                cpu->addr_abs = cpu->pc + cpu->addr_rel;

                // If the memory page is different, then incur another clock
                // cycle.
                if ((cpu->addr_abs & 0xFF00) != (cpu->pc & 0xFF00)) {
                        cpu->cycles++;
                }

                // Set the program counter.
                cpu->pc = cpu->addr_abs;
        }

        return 0;
}

//! \brief Bit test operation
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t BIT(struct cpu *cpu) {
        uint16_t fetched = Fetch(cpu);
        uint16_t tmp = cpu->a & fetched;
        SetFlag(cpu, Z, (tmp & 0x00FF) == 0x00);
        SetFlag(cpu, N, fetched & (1 << 7));
        SetFlag(cpu, V, fetched & (1 << 6));
        return 0;
}

//! \brief Branch if Negative
//!
//! if (N == 1) pc = address
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t BMI(struct cpu *cpu) {
        if (GetFlag(cpu, N) == 1) {
                cpu->cycles++;
                cpu->addr_abs = cpu->pc + cpu->addr_rel;

                if ((cpu->addr_abs & 0xFF00) != (cpu->pc & 0xFF00)) {
                        cpu->cycles++;
                }

                cpu->pc = cpu->addr_abs;
        }
        return 0;
}

//! \brief Branch if Not Equal
//!
//! if (Z == 0) pc = address
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t BNE(struct cpu *cpu) {
        if (GetFlag(cpu, Z) == 0) {
                cpu->cycles++;
                cpu->addr_abs = cpu->pc + cpu->addr_rel;

                if ((cpu->addr_abs & 0xFF00) != (cpu->pc & 0xFF00)) {
                        cpu->cycles++;
                }

                cpu->pc = cpu->addr_abs;
        }
        return 0;
}

//! \brief Branch if Positive
//!
//! if (N == 0) pc = address
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t BPL(struct cpu *cpu) {
        if (GetFlag(cpu, N) == 0) {
                cpu->cycles++;
                cpu->addr_abs = cpu->pc + cpu->addr_rel;

                if ((cpu->addr_abs & 0xFF00) != (cpu->pc & 0xFF00)) {
                        cpu->cycles++;
                }

                cpu->pc = cpu->addr_abs;
        }
        return 0;
}

//! \brief Break
//!
//! //! TODO
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t BRK(struct cpu*cpu) {
        cpu->pc++;

        SetFlag(cpu, I, 1);
        BusWrite(cpu->bus, 0x0100 + cpu->sp, (cpu->pc >> 8) & 0x00FF);
        cpu->sp--;
        BusWrite(cpu->bus, 0x0100 + cpu->sp, cpu->pc & 0x00FF);
        cpu->sp--;

        SetFlag(cpu, B, 1);
        BusWrite(cpu->bus, 0x0100 + cpu->sp, cpu->status);
        cpu->sp--;
        SetFlag(cpu, B, 0);

        cpu->pc = (uint16_t)BusRead(cpu->bus, 0xFFFE) | ((uint16_t)BusRead(cpu->bus, 0xFFFF) << 8);
        return 0;
}

//! \brief Branch if Overflow Clear
//!
//! if (V == 0) pc = address
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t BVC(struct cpu *cpu) {
        if (GetFlag(cpu, V) == 0) {
                cpu->cycles++;
                cpu->addr_abs = cpu->pc + cpu->addr_rel;

                if ((cpu->addr_abs & 0xFF00) != (cpu->pc & 0xFF00)) {
                        cpu->cycles++;
                }

                cpu->pc = cpu->addr_abs;
        }
        return 0;
}

//! \brief Branch if Overflow Set
//!
//! if (V == 1) pc = address
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t BVS(struct cpu *cpu) {
        if (GetFlag(cpu, V) == 1) {
                cpu->cycles++;
                cpu->addr_abs = cpu->pc + cpu->addr_rel;

                if ((cpu->addr_abs & 0xFF00) != (cpu->pc & 0xFF00)) {
                        cpu->cycles++;
                }

                cpu->pc = cpu->addr_abs;
        }
        return 0;
}

//! \brief Clear Carry Flag
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t CLC(struct cpu *cpu) {
        SetFlag(cpu, C, false);
        return 0;
}

//! \brief Clear Decimal Flag
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t CLD(struct cpu *cpu) {
        SetFlag(cpu, D, false);
        return 0;
}

//! \brief Disable Interrupts / Clear Interrupt Flag
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t CLI(struct cpu *cpu) {
        SetFlag(cpu, I, false);
        return 0;
}

//! \brief Clear Overflow Flag
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t CLV(struct cpu *cpu) {
        SetFlag(cpu, V, false);
        return 0;
}

//! \brief Compare Accumulator
//!
//! Carry = A >= data
//! Zero = (A - data) == 0
//!
//! \param[in,out] cpu
//! \return 1 This instruction will incur another cycle if the addressing mode
//! demands it
uint8_t CMP(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        uint16_t tmp = (uint16_t)cpu->a - (uint16_t)fetched;
        SetFlag(cpu, C, cpu->a >= fetched);
        SetFlag(cpu, Z, (tmp & 0x00FF) == 0x0000);
        SetFlag(cpu, N, tmp & 0x0080);
        return 1;
}

//! \brief Compare X Register
//!
//! Carry = X >= data
//! Zero = (X - data) == 0
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t CPX(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        uint16_t tmp = (uint16_t)cpu->x - (uint16_t)fetched;
        SetFlag(cpu, C, cpu->x >= fetched);
        SetFlag(cpu, Z, (tmp & 0x00FF) == 0x0000);
        SetFlag(cpu, N, tmp & 0x0080);
        return 0;
}

//! \brief Compare Y Register
//!
//! Carry = Y >= data
//! Zero = (Y - data) == 0
//! Flags affected: C,Z,N
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t CPY(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        uint16_t tmp = (uint16_t)cpu->y - (uint16_t)fetched;
        SetFlag(cpu, C, cpu->y >= fetched);
        SetFlag(cpu, Z, (tmp & 0x00FF) == 0x0000);
        SetFlag(cpu, N, tmp & 0x0080);
        return 0;
}

//! \brief Decrement Value at Memory Location
//!
//! Data = Data - 1
//! Flags affected: N, Z
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t DEC(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        uint16_t tmp = fetched - 1;
        BusWrite(cpu->bus, cpu->addr_abs, tmp & 0x00FF);

        SetFlag(cpu, Z, (tmp & 0x00FF) == 0x0000);
        SetFlag(cpu, N, tmp & 0x0080);
        return 0;
}

//! \brief Decrement X Register
//!
//! X = X - 1
//! Flags affected: N, Z
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t DEX(struct cpu *cpu) {
        cpu->x--;

        SetFlag(cpu, Z, cpu->x == 0x00);
        SetFlag(cpu, N, cpu->x & 0x80);
        return 0;
}

//! \brief Decrement Y Register
//!
//! Y = Y - 1
//! Flags affected: N, Z
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t DEY(struct cpu *cpu) {
        cpu->y--;

        SetFlag(cpu, Z, cpu->y == 0x00);
        SetFlag(cpu, N, cpu->y & 0x80);
        return 0;
}

//! \brief Bitwise Logic XOR
//!
//! A = A XOR M
//! Flags affected: N, Z
//!
//! \param[in,out] cpu
//! \return 1 This instruction will incur another cycle if the addressing mode
//! demands it
uint8_t EOR(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        cpu->a = cpu->a ^ fetched;

        SetFlag(cpu, Z, cpu->a == 0x00);
        SetFlag(cpu, N, cpu->a & 0x80);
        return 1;
}

//! \brief Increment Value at Memory Location
//!
//! Data = Data + 1
//! Flags affected: N, Z
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t INC(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        uint16_t tmp = fetched + 1;
        BusWrite(cpu->bus, cpu->addr_abs, tmp & 0x00FF);

        SetFlag(cpu, Z, (tmp & 0x00FF) == 0x0000);
        SetFlag(cpu, N, tmp & 0x0080);
        return 0;
}

//! \brief Increment X Register
//!
//! X = X + 1
//! Flags affected: N, Z
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t INX(struct cpu *cpu) {
        cpu->x++;

        SetFlag(cpu, Z, cpu->x == 0x00);
        SetFlag(cpu, N, cpu->x & 0x80);
        return 0;
}

//! \brief Increment Y Register
//!
//! Y = Y + 1
//! Flags affected: N, Z
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t INY(struct cpu *cpu) {
        cpu->y++;

        SetFlag(cpu, Z, cpu->y == 0x00);
        SetFlag(cpu, N, cpu->y & 0x80);
        return 0;
}

//! \brief Jump to Location
//!
//! pc = address
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t JMP(struct cpu *cpu) {
        cpu->pc = cpu->addr_abs;
        return 0;
}

//! \brief Jump to Sub-Routine
//!
//! push(sp, pc), pc = address
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t JSR(struct cpu *cpu) {
        cpu->pc--;

        BusWrite(cpu->bus, 0x0100 + cpu->sp, (cpu->pc >> 8) & 0x00FF);
        cpu->sp--;

        BusWrite(cpu->bus, 0x0100 + cpu->sp, cpu->pc & 0x00FF);
        cpu->sp--;

        cpu->pc = cpu->addr_abs;
        return 0;
}

//! \brief Load Into Accumulator
//!
//! A = data
//!
//! \param[in,out] cpu
//! \return 1 This instruction will incur another cycle if the addressing mode
//! demands it
uint8_t LDA(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        cpu->a = fetched;
        SetFlag(cpu, Z, cpu->a == 0x00);
        SetFlag(cpu, N, cpu->a & 0x80);
        return 1;
}

//! \brief Load Into X Register
//!
//! X = data
//!
//! \param[in,out] cpu
//! \return 1 This instruction will incur another cycle if the addressing mode
//! demands it
uint8_t LDX(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        cpu->x = fetched;
        SetFlag(cpu, Z, cpu->x == 0x00);
        SetFlag(cpu, N, cpu->x & 0x80);
        return 1;
}

//! \brief Load Into Y Register
//!
//! Y = data
//!
//! \param[in,out] cpu
//! \return 1 This instruction will incur another cycle if the addressing mode
//! demands it
uint8_t LDY(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        cpu->y = fetched;
        SetFlag(cpu, Z, cpu->y == 0x00);
        SetFlag(cpu, N, cpu->y & 0x80);
        return 1;
}

//! \brief ??
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t LSR(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        SetFlag(cpu, C, fetched & 0x0001);
        uint16_t tmp = fetched >> 1;
        SetFlag(cpu, Z, (tmp & 0x00FF) == 0x0000);
        SetFlag(cpu, N, tmp & 0x0080);

        if (instruction_map[cpu->opcode].address == IMP)
                cpu->a = tmp & 0x00FF;
        else
                BusWrite(cpu->bus, cpu->addr_abs, tmp & 0x00FF);

        return 0;
}

//! \brief Standard NOP
//!
//! Unfortunately, not all NOPS are equal.
//! See: https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t NOP(struct cpu *cpu) {
        switch(cpu->opcode) {
                case 0x1C:
                case 0x3C:
                case 0x5C:
                case 0x7C:
                case 0xDC:
                case 0xFC:
                        return 1;
                        break;
        }
        return 0;
}

//! \brief Bitwise Logical OR
//!
//! A = A | M
//!
//! \param[in,out] cpu
//! \return 1 This instruction will incur another cycle if the addressing mode
//! demands it
uint8_t ORA(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        cpu->a = cpu->a | fetched;
        SetFlag(cpu, Z, cpu->a == 0x00);
        SetFlag(cpu, N, cpu->a & 0x80);
        return 1;
}

//! \brief Push Accumulator to Stack
//!
//! push(sp, A)
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t PHA(struct cpu *cpu) {
        BusWrite(cpu->bus, 0x0100 + cpu->sp, cpu->a);
        cpu->sp--;
        return 0;
}

//! \brief Push Status Register to Stack
//!
//! push(sp, status)
//!
//! NOTE: Break flag is set to 1 before push
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t PHP(struct cpu *cpu) {
        BusWrite(cpu->bus, 0x0100 + cpu->sp, cpu->status | B | U);
        SetFlag(cpu, B, 0);
        SetFlag(cpu, U, 0);
        cpu->sp--;
        return 0;
}

//! \brief Pop Accumulator off Stack
//!
//! A = pop(sp)
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t PLA(struct cpu *cpu) {
        cpu->sp++;
        cpu->a = BusRead(cpu->bus, 0x0100 + cpu->sp);
        SetFlag(cpu, Z, cpu->a == 0x00);
        SetFlag(cpu, N, cpu->a & 0x80);
        return 0;
}

//! \brief Pop status Register off Stack
//!
//! status = pop(sp)
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t PLP(struct cpu *cpu) {
        cpu->sp++;
        cpu->status = BusRead(cpu->bus, 0x0100 + cpu->sp);
        SetFlag(cpu, U, 1);
        return 0;
}

//! \brief ??
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t ROL(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        uint16_t tmp = (uint16_t)(fetched << 1) | GetFlag(cpu, C);
        SetFlag(cpu, C, tmp & 0xFF00);
        SetFlag(cpu, Z, (tmp & 0x00FF) == 0x0000);
        SetFlag(cpu, N, tmp & 0x0080);

        if (instruction_map[cpu->opcode].address == IMP)
                cpu->a = tmp & 0x00FF;
        else
                BusWrite(cpu->bus, cpu->addr_abs, tmp & 0x00FF);

        return 0;

}

//! \brief ??
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t ROR(struct cpu *cpu) {
        uint8_t fetched = Fetch(cpu);
        uint16_t tmp = (uint16_t)(GetFlag(cpu, C) << 7) | (fetched >> 1);
        SetFlag(cpu, C, fetched & 0x01);
        SetFlag(cpu, Z, (tmp & 0x00FF) == 0x0000);
        SetFlag(cpu, N, tmp & 0x0080);

        if (instruction_map[cpu->opcode].address == IMP)
                cpu->a = tmp & 0x00FF;
        else
                BusWrite(cpu->bus, cpu->addr_abs, tmp & 0x00FF);

        return 0;
}

//! \brief Return From Interrupt
//!
//! Restore cpu state and status flags after processing an interrupt.
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t RTI(struct cpu *cpu) {
        cpu->sp++;
        cpu->status = BusRead(cpu->bus, 0x0100 + cpu->sp);
        cpu->status &= ~B;
        cpu->status &= ~U;

        cpu->sp++;
        cpu->pc = (uint16_t)BusRead(cpu->bus, 0x0100 + cpu->sp);
        cpu->sp++;
        cpu->pc |= (uint16_t)BusRead(cpu->bus, 0x0100 + cpu->sp) << 8;

        return 0;
}

//! \brief ??
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t RTS(struct cpu *cpu) {
        cpu->sp++;
        cpu->pc = (uint16_t)BusRead(cpu->bus, 0x0100 + cpu->sp);
        cpu->sp++;
        cpu->pc |= (uint16_t)BusRead(cpu->bus, 0x0100 + cpu->sp) << 8;

        cpu->pc++;
        return 0;
}

//! \brief Set Carry Flag
//!
//! C = 1
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t SEC(struct cpu *cpu) {
        SetFlag(cpu, C, true);
        return 0;
}

//! \brief Set Decimal Flag
//!
//! D = 1
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t SED(struct cpu *cpu) {
        SetFlag(cpu, D, true);
        return 0;
}

//! \brief Set Interrupt Flag / Enable Interrupts
//!
//! I = 1
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t SEI(struct cpu *cpu) {
        SetFlag(cpu, I, true);
        return 0;
}

//! \brief Store Accumulator at Address
//!
//! data = A
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t STA(struct cpu *cpu) {
        BusWrite(cpu->bus, cpu->addr_abs, cpu->a);
        return 0;
}

//! \brief Store X Register at Address
//!
//! data = X
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t STX(struct cpu *cpu) {
        BusWrite(cpu->bus, cpu->addr_abs, cpu->x);
        return 0;
}

//! \brief Store Y Register at Address
//!
//! data = Y
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t STY(struct cpu *cpu) {
        BusWrite(cpu->bus, cpu->addr_abs, cpu->y);
        return 0;
}

//! \brief Trasnfer Accumulator to X Register
//!
//! X = A
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t TAX(struct cpu *cpu) {
        cpu->x = cpu->a;
        SetFlag(cpu, Z, cpu->x == 0x00);
        SetFlag(cpu, N, cpu->x & 0x80);
        return 0;
}

//! \brief Transfer Accumulator to Y Register
//!
//! Y = A
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t TAY(struct cpu *cpu) {
        cpu->y = cpu->a;
        SetFlag(cpu, Z, cpu->y == 0x00);
        SetFlag(cpu, N, cpu->y & 0x80);
        return 0;
}

//! \brief Transfer Stack Pointer to X Register
//!
//! X = sp
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t TSX(struct cpu *cpu) {
        cpu->x = cpu->sp;
        SetFlag(cpu, Z, cpu->x == 0x00);
        SetFlag(cpu, N, cpu->x & 0x80);
        return 0;
}

//! \brief Transfer X Register to Accumulator
//!
//! A = X
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t TXA(struct cpu *cpu) {
        cpu->a = cpu->x;
        SetFlag(cpu, Z, cpu->a == 0x00);
        SetFlag(cpu, N, cpu->a & 0x80);
        return 0;
}

//! \brief Transfer X Register to Stack Pointer
//!
//! sp = X
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t TXS(struct cpu *cpu) {
        cpu->sp = cpu->x;
        return 0;
}

//! \brief Transfer Y Register to Accumulator
//!
//! A = Y
//!
//! \param[in,out] cpu
//! \return 0 This instruction will take no additional cycles
uint8_t TYA(struct cpu *cpu) {
        cpu->a = cpu->y;
        SetFlag(cpu, Z, cpu->a == 0x00);
        SetFlag(cpu, N, cpu->a & 0x80);
        return 0;
}

//! \brief All illegal opcodes route here
//! \return 0 This instruction will take no additional cycles
uint8_t XXX(struct cpu *cpu) {
        return 0;
}


//-- Helper Functions ----------------------------------------------------------


//! \brief convert number to hexadecimal string
//!
//! \param[in] n number to convert to hex
//! \param[in] d number of nibbles in data source
//! \param[in,out] buf pre-allocated char buffer
//! \param[in] size size of buf
void ToHexString(uint32_t n, uint8_t d, char *buf, uint8_t size) {
        if (d > size) {
                d = size;
        }

        for (int i = d - 1; i >= 0; i--, n >>= 4) {
                buf[i] = "0123456789ABCDEF"[n & 0xF];
        }

        buf[d] = '\0';
}

void DebugInstructionMapDeinit(struct debug_instruction_map *map) {
        if (NULL == map) {
                return;
        }

        if (NULL != map->map) {
                free(map->map);
        }

        free(map);
}

struct debug_instruction *DebugInstructionInit(uint16_t address, char *string, int length) {
        struct debug_instruction *self = (struct debug_instruction *)malloc(sizeof(struct debug_instruction));
        self->text = malloc(length);
        strncpy(self->text, string, length);
        return self;
}

void DebugInstructionDeinit(struct debug_instruction *self) {
        if (NULL == self) {
                return;
        }

        if (NULL != self->text) {
                free(self->text);
        }

        free(self);
}

int CpuIsComplete(struct cpu *cpu) {
        return (0 == cpu->cycles);
}

struct debug_instruction_map *CpuDisassemble(struct cpu *cpu, uint16_t start, uint16_t stop) {
        int addr = start;
        uint8_t value = 0x00;
        uint8_t lo = 0x00;
        uint8_t hi = 0x00;
        uint16_t line_addr = 0;

        struct debug_instruction_map *debug_map = (struct debug_instruction_map *)malloc(sizeof(struct debug_instruction_map));
        debug_map->count = stop - start;
        debug_map->map = (struct debug_instruction **)malloc(sizeof(struct debug_instruction *) * debug_map->count);

        char text[256] = { 0 };
        char text_cpy[256] = { 0 };
        uint8_t hex_buf_len = 5; // Space for terminating null.
        char hex_buf[hex_buf_len];
        uint16_t text_len = hex_buf_len; // Size adjusts as we construct string below.

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wformat-truncation"

        int i = 0;
        while (addr <= stop) {
                text_len = hex_buf_len; // Size adjusts as we construct string below.
                line_addr = addr;
                struct instruction instruction;

                // Prefix instruction with address.
                ToHexString(addr, 4, hex_buf, hex_buf_len);
                text_len += 3; // Adding $, <colon> and <space>
                snprintf(text, text_len, "$%s: ", hex_buf);
                strncpy(text_cpy, text, strnlen(text, text_len) + 1);

                // Get the readable name of the instruction.
                uint8_t opcode = BusRead(cpu->bus, addr);
                instruction = instruction_map[opcode];
                addr++;
                text_len += 4; // instruction.name is 3 chars, plus an extra space.
                snprintf(text, text_len, "%s%s ", text_cpy, instruction.name); //<--
                strncpy(text_cpy, text, strnlen(text, text_len));

                if (IMP == instruction.address) {
                        snprintf(text, 256, "%s {IMP}", text_cpy);
                } else if (IMM == instruction.address) {
                        value = BusRead(cpu->bus, addr);
                        addr++;
                        ToHexString(value, 2, hex_buf, hex_buf_len);
                        snprintf(text, 256, "%s#$%s {IMM}", text_cpy, hex_buf);
                } else if (ZP0 == instruction.address) {
                        lo = BusRead(cpu->bus, addr);
                        addr++;
                        hi = 0x00;
                        ToHexString(lo, 2, hex_buf, hex_buf_len);
                        snprintf(text, 256, "%s$%s {ZP0}", text_cpy, hex_buf);
                } else if (ZPX == instruction.address) {
                        lo = BusRead(cpu->bus, addr);
                        addr++;
                        hi = 0x00;
                        ToHexString(lo, 2, hex_buf, hex_buf_len);
                        snprintf(text, 256, "%s$%s, X {ZPX}", text_cpy, hex_buf);
                } else if (ZPY == instruction.address) {
                        lo = BusRead(cpu->bus, addr);
                        addr++;
                        hi = 0x00;
                        ToHexString(lo, 2, hex_buf, hex_buf_len);
                        snprintf(text, 256, "%s$%s, Y {ZPY}", text_cpy, hex_buf);
                } else if (IZX == instruction.address) {
                        lo = BusRead(cpu->bus, addr);
                        addr++;
                        hi = 0x00;
                        ToHexString(lo, 2, hex_buf, hex_buf_len);
                        snprintf(text, 256, "%s($%s, X) {IZX}", text_cpy, hex_buf);
                } else if (IZY == instruction.address) {
                        lo = BusRead(cpu->bus, addr);
                        addr++;
                        hi = 0x00;
                        ToHexString(lo, 2, hex_buf, hex_buf_len);
                        snprintf(text, 256, "%s($%s, Y) {IZY}", text_cpy, hex_buf);
                } else if (ABS == instruction.address) {
                        lo = BusRead(cpu->bus, addr);
                        addr++;
                        hi = BusRead(cpu->bus, addr);
                        addr++;
                        ToHexString((uint16_t)(hi << 8) | lo, 4, hex_buf, hex_buf_len);
                        snprintf(text, 256, "%s$%s {ABS}", text_cpy, hex_buf);
                } else if (ABX == instruction.address) {
                        lo = BusRead(cpu->bus, addr);
                        addr++;
                        hi = BusRead(cpu->bus, addr);
                        addr++;
                        ToHexString((uint16_t)(hi << 8) | lo, 4, hex_buf, hex_buf_len);
                        snprintf(text, 256, "%s$%s, X {ABX}", text_cpy, hex_buf);
                } else if (ABY == instruction.address) {
                        lo = BusRead(cpu->bus, addr);
                        addr++;
                        hi = BusRead(cpu->bus, addr);
                        addr++;
                        ToHexString((uint16_t)(hi << 8) | lo, 4, hex_buf, hex_buf_len);
                        snprintf(text, 256, "%s$%s, Y {ABY}", text_cpy, hex_buf);
                } else if (IND == instruction.address) {
                        lo = BusRead(cpu->bus, addr);
                        addr++;
                        hi = BusRead(cpu->bus, addr);
                        addr++;
                        ToHexString((uint16_t)(hi << 8) | lo, 4, hex_buf, hex_buf_len);
                        snprintf(text, 256, "%s($%s) {IND}", text_cpy, hex_buf);
                } else if (REL == instruction.address) {
                        value = BusRead(cpu->bus, addr);
                        addr++;
                        ToHexString(value, 2, hex_buf, hex_buf_len);

                        char hex_buf2[5];
                        ToHexString(addr + (int8_t)value, 4, hex_buf2, hex_buf_len);
                        snprintf(text, 256, "%s$%s [$%s] {REL}", text_cpy, hex_buf, hex_buf2);
                }

                debug_map->map[i] = DebugInstructionInit(line_addr, text, strnlen(text, 256));
                i++;
        }

        #pragma GCC diagnostic pop

        return debug_map;
}
