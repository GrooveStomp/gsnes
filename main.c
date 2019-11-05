/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: main.c
  Created: 2019-10-31
  Updated: 2019-10-31
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
#include <stdlib.h> // strtoul
#include <string.h> // strlen
#include <stdio.h> // printf

#include "cpu.h"
#include "bus.h"

//! \file main.c

// Load Program (assembled at https://www.masswerk.at/6502/assembler.html)
/*
 *=$8000
 LDX #10
 STX $0000
 LDX #3
 STX $0001
 LDY $0000
 LDA #0
 CLC
 loop
 ADC $0001
 DEY
 BNE loop
 STA $0002
 NOP
 NOP
 NOP
*/
static char *program = "A2 0A 8E 00 00 A2 03 8E 01 00 AC 00 00 A9 00 18 6D 01 00 88 D0 FA 8D 02 00 EA EA EA";

void LoadProgram(struct bus *bus, char *data) {
        uint16_t ram_offset = 0;
        char *last_byte = NULL;

        unsigned long i = strtoul(data, &last_byte, 16);
        while (data != last_byte) {
                BusWrite(bus, ram_offset++, (uint8_t)i);
                data = last_byte;
                i = strtoul(data, &last_byte, 16);
        }
}

int main(int argc, char **argv) {
        struct cpu *cpu = CpuInit();
        struct bus *bus = BusInit(cpu);
        CpuConnectBus(cpu, bus);

        LoadProgram(bus, program);

        // Set reset vector.
        BusWrite(bus, 0xFFFC, 0x00);
        BusWrite(bus, 0xFFFD, 0x80);

        // Disassemble
        struct debug_instruction_map *map = CpuDisassemble(cpu, 0x0000, 0x000F);

        for (int i = 0; i < map->count; i++) {
                printf("map{%p}", (void *)map);
                if (NULL != map) {
                        printf("->map{%p}", (void *)map->map);

                        if (NULL != map->map) {
                                printf("[i]{%p}", (void *)map->map[i]);

                                if (NULL != map->map[i]) {
                                        printf("->text{%s}", map->map[i]->text);
                                }
                        }
                }
                printf("\n");
        }

        DebugInstructionMapDeinit(map);

        // Reset
        CpuReset(cpu);

        BusDeinit(bus);
        CpuDeinit(cpu);

        return 0;
}
