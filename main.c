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
//! \file main.c
#include <time.h> // nanosleep
#include <stdlib.h> // strtoul, exit
#include <string.h> // strlen
#include <stdio.h> // printf

#include "cpu.h"
#include "bus.h"
#include "ppu.h"
#include "graphics.h"
#include "input.h"

//! \brief convert from seconds to nanoseconds
//!
//! Used to make it easier to understand the value being passed to nanosleep()
//!
//! \param x number of seconds
//! \return x as nanoseconds
#define S_TO_NS(x) (x) * 1000000000

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

static int WIDTH = 1200;
static int HEIGHT = 800;

static struct cpu *cpu = NULL;
static struct ppu *ppu = NULL;
static struct bus *bus = NULL;
static struct input *input = NULL;
static struct graphics *graphics = NULL;
static char *font_buffer = NULL;

void Deinit(int code) {
        if (NULL != font_buffer)
                free(font_buffer);
        if (NULL != graphics)
                GraphicsDeinit(graphics);
        if (NULL != input)
                InputDeinit(input);
        if (NULL != ppu)
                PpuDeinit(ppu);
        if (NULL != bus)
                BusDeinit(bus);
        if (NULL != cpu)
                CpuDeinit(cpu);

        exit(code);
}

void Init() {
        char *ttf_filename = "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf";

        cpu = CpuInit();
        if (NULL == cpu) {
                fprintf(stderr, "Couldn't initialize cpu");
                Deinit(1);
        }

        bus = BusInit(cpu);
        if (NULL == bus) {
                fprintf(stderr, "Couldn't initialize bus");
                Deinit(1);
        }

        input = InputInit();
        if (NULL == input) {
                fprintf(stderr, "Couldn't initialize input");
                Deinit(1);
        }

        graphics = GraphicsInit("GrooveStomp's NES Emulator", WIDTH, HEIGHT);
        if (NULL == graphics) {
                fprintf(stderr, "Couldn't initialize graphics");
                Deinit(1);
        }

        FILE *ttf_file = fopen(ttf_filename, "rb");
        if (NULL == ttf_file) {
                perror("fopen() failed");
                fclose(ttf_file);
                Deinit(1);
        }

        fseek(ttf_file, 0, SEEK_END);
        size_t fsize = ftell(ttf_file);
        font_buffer = (char *)malloc(fsize);
        if (NULL == font_buffer) {
                fprintf(stderr, "Couldn't allocate space for font buffer");
                fclose(ttf_file);
                Deinit(1);
        }

        fseek(ttf_file, 0, SEEK_SET);
        size_t objs_read = fread(font_buffer, 1, fsize, ttf_file);
        if (ferror(ttf_file)) {
                perror("fread() failed");
                fclose(ttf_file);
                Deinit(1);
        }
        fclose(ttf_file);
        GraphicsInitText(graphics, (unsigned char *)font_buffer);
}

void LoadRom(struct bus *bus, char *data) {
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
        Init();
        struct timespec sleep = { .tv_sec = 0, .tv_nsec = S_TO_NS(0.0333) };

        CpuConnectBus(cpu, bus);
        LoadRom(bus, program);

        // Set reset vector.
        BusWrite(bus, 0xFFFC, 0x00);
        BusWrite(bus, 0xFFFD, 0x80);

        // Disassemble
        struct debug_instruction_map *map = CpuDisassemble(cpu, 0x0000, 0x000F);

        /* for (int i = 0; i < map->count; i++) { */
        /*         printf("map{%p}", (void *)map); */
        /*         if (NULL != map) { */
        /*                 printf("->map{%p}", (void *)map->map); */

        /*                 if (NULL != map->map) { */
        /*                         printf("[i]{%p}", (void *)map->map[i]); */

        /*                         if (NULL != map->map[i]) { */
        /*                                 printf("->text{%s}", map->map[i]->text); */
        /*                         } */
        /*                 } */
        /*         } */
        /*         printf("\n"); */
        /* } */

        int running = 1;
        while (running) {
                GraphicsBegin(graphics);
                GraphicsClearScreen(graphics, 0x000000FF);
                GraphicsDrawText(graphics, 50, 50, "Hello", 40, 0xFFFFFFFF);
                GraphicsEnd(graphics);

                InputProcess(input);
                running = !InputIsQuitRequested(input);

                nanosleep(&sleep, NULL);
        }

        DebugInstructionMapDeinit(map);

        // Reset
        CpuReset(cpu);

        Deinit(0);
        return 0;
}
