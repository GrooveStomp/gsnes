/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: main.c
  Created: 2019-10-31
  Updated: 2019-10-31
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file main.c
#include <time.h> // struct timespec, clock_gettime
#include <stdlib.h> // strtoul, exit
#include <string.h> // strlen
#include <stdio.h> // printf

#include "cpu.h"
#include "bus.h"
#include "ppu.h"
#include "graphics.h"
#include "input.h"
#include "util.h"
#include "color.h"

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
        char *ttf_filename = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";

        cpu = CpuInit();
        if (NULL == cpu) {
                fprintf(stderr, "Couldn't initialize cpu");
                Deinit(1);
        }

        ppu = PpuInit();
        if (NULL == ppu) {
                fprintf(stderr, "Couldn't initialize ppu");
                Deinit(1);
        }

        bus = BusInit(cpu, ppu);
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
        size_t objsRead = fread(font_buffer, 1, fsize, ttf_file);
        if (objsRead != 1 && ferror(ttf_file)) {
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

void DrawCpuState(int x, int y) {
        GraphicsDrawText(graphics, x, y, "CPU State", 25, 0x000000FF);
        char **cpu_state = CpuDebugStateInit(cpu);
        for (int i = 0; i < 7; i++) {
                GraphicsDrawText(graphics, x, (y - 20) - (18 * i), cpu_state[i], 15, 0x000000FF);
        }
        CpuDebugStateDeinit(cpu_state);
}

void DrawDisassembly(struct disassembly *disassembly, int x, int y, int numLines) {
        GraphicsDrawText(graphics, x, y, "Disassembly", 25, 0x000000FF);

        int pc = DisassemblyFindPc(disassembly, cpu);

        int halfLines = (int)(0.5f * (float)numLines);
        int min = pc - halfLines;
        if (min < 0 || min > 0xFFFF) min = 0;

        int max = pc + halfLines;
        if (max > 0xFFFF || max < 0) max = 0xFFFF;

        for (int i = min; i < max; i++) {
                if (NULL == disassembly || NULL == disassembly->map || NULL == disassembly->map[i]) {
                        GraphicsDrawLine(graphics, x, (y - 25) - (9 * i), x + 100, (y - 15) - (9 * i), ColorBlack.rgba);
                } else if (i == pc) {
                        GraphicsDrawText(graphics, x, (y - 25) - (18 * i), disassembly->map[i]->text, 15, ColorBlue.rgba);
                } else {
                        GraphicsDrawText(graphics, x, (y - 25) - (18 * i), disassembly->map[i]->text, 15, ColorBlack.rgba);
                }
        }
}

int main(int argc, char **argv) {
        Init();

        CpuConnectBus(cpu, bus);
        LoadRom(bus, program);

        // Set reset vector.
        BusWrite(bus, 0xFFFC, 0x00);
        BusWrite(bus, 0xFFFD, 0x80);

        // Disassemble
        struct disassembly *disassembly = DisassemblyInit(cpu, 0x0000, 0x00FF);

        struct timespec frameEnd;
        struct timespec frameStart;
        clock_gettime(CLOCK_REALTIME, &frameStart);

        double residualTime = 0.0;
        int isEmulating = 1;
        int isRunning = 1;
        while (isRunning) {
                clock_gettime(CLOCK_REALTIME, &frameEnd);
                double elapsedTime = S_AS_MS(frameEnd.tv_sec - frameStart.tv_sec);
                elapsedTime += NS_AS_MS(frameEnd.tv_nsec - frameStart.tv_nsec);

                GraphicsBegin(graphics);
                GraphicsClearScreen(graphics, 0xFFFFFFFF);

                if (isEmulating) {
                        if (0.0f < residualTime) {
                                residualTime -= elapsedTime;
                        } else {
                                residualTime += (1.0 / 60.0) - elapsedTime;
                                do { BusTick(bus); } while (!PpuIsFrameComplete(ppu));
                                PpuResetFrameCompletion(ppu);
                        }
                } else {
                        // Emulate code step-by-step.
                        if (InputIsKeyPressed(input, KEY_C)) {
                                // Tick enough times to execute a whole CPU instruction.
                                do { BusTick(bus); } while (!CpuIsComplete(cpu));

                                // The CPU clock runs slower than system clock,
                                // so it may be incomplete for additional system
                                // clock cycles. Drain those out.
                                do { BusTick(bus); } while (CpuIsComplete(cpu));
                        }

                        // Emulate one whole frame.
                        if (InputIsKeyPressed(input, KEY_F)) {
                                // Clock enough times to draw a single frame.
                                do { BusTick(bus); } while (!PpuIsFrameComplete(ppu));

                                // Use residual clock cycles to complete the
                                // current instruction.
                                do { BusTick(bus); } while (!CpuIsComplete(cpu));

                                // Reset frame completion flag.
                                PpuResetFrameCompletion(ppu);
                        }
                }

                InputProcess(input);
                isRunning = !InputIsQuitRequested(input);

                if (InputIsKeyPressed(input, KEY_SPACE)) isEmulating = !isEmulating;
                if (InputIsKeyPressed(input, KEY_R)) BusReset(bus);

                GraphicsDrawLine(graphics, 811, 0, 811, 800, 0x000000FF);
                DrawCpuState(820, 775);

                GraphicsDrawLine(graphics, 811, 630, 1200, 630, 0x000000FF);
                DrawDisassembly(disassembly, 820, 600, 25);

                GraphicsEnd(graphics);
        }

        DisassemblyDeinit(disassembly);

        // Reset
        BusReset(bus);

        Deinit(0);
        return 0;
}
