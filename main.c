/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: main.c
  Created: 2019-10-31
  Updated: 2019-12-16
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file main.c
#include <math.h> // sin
#include <stdio.h> // printf
#include <stdlib.h> // strtoul, exit
#include <string.h> // strlen
#include <time.h> // struct timespec, clock_gettime

#include <pthread.h>

#include "bus.h"
#include "cart.h"
#include "color.h"
#include "cpu.h"
#include "graphics.h"
#include "input.h"
#include "ppu.h"
#include "sound.h"
#include "util.h"

static const int FONT_HEADER_SCALE = 20;
static const int FONT_SCALE = 15;

static const int NES_SCREEN_WIDTH = 256 * 3;
static const int NES_SCREEN_HEIGHT = 240 * 3;
static const int WIDTH = NES_SCREEN_WIDTH + 250;
static const int HEIGHT = NES_SCREEN_HEIGHT;
static const int SWATCH_SIZE = 5;

static struct cpu *cpu = NULL;
static struct ppu *ppu = NULL;
static struct bus *bus = NULL;
static struct input *input = NULL;
static struct graphics *graphics = NULL;
static struct cart *cart = NULL;
static struct sound *sound = NULL;
static char *font_buffer = NULL;

float SynthFn(int numChannels, float timeElapsedS, float timeStemp);

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
        if (NULL != cart)
                CartDeinit(cart);

        if (NULL != sound)
                SoundDeinit(sound);

        exit(code);
}

void Init() {
        char *ttf_filename = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";

        cart = CartInit("super_mario_bros.nes");
        if (NULL == cart) {
                fprintf(stderr, "Couldn't load cart");
                Deinit(1);
        }
        if (!CartIsImageValid(cart)) {
                fprintf(stderr, "Couldn't load cart");
                Deinit(1);
        }

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

        sound = SoundInit(44100, 1);
        if (NULL == sound) {
                fprintf(stderr, "Couldn't initialize sound");
                Deinit(1);
        }
        SoundSetSynthFn(SynthFn);

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

void DrawCpuState(int x, int y) {
        GraphicsDrawText(graphics, x, y, "CPU State", FONT_HEADER_SCALE, 0x000000FF);
        int numLines = 0;
        char **cpuState = CpuDebugStateInit(cpu, &numLines);
        for (int i = 0; i < numLines; i++) {
                GraphicsDrawText(graphics, x, (y - 15) - (18 * i), cpuState[i], FONT_SCALE, 0x000000FF);
        }
        CpuDebugStateDeinit(cpuState);
}

void DrawDisassembly(struct disassembly *disassembly, int x, int y, int numLines) {
        if (NULL == disassembly) return;

        GraphicsDrawText(graphics, x, y, "Disassembly", FONT_HEADER_SCALE, 0x000000FF);

        int pc = DisassemblyFindPc(disassembly, cpu);

        int halfLines = (int)(0.5f * (float)numLines);
        int min = pc - halfLines;
        int max = pc + halfLines;

        for (int i = min, si = 0; i < max; i++, si++) {
                int yOff = (y - 25) - (18 * si);
                if (i < 0 || i > 0xFFFF) {
                        GraphicsDrawLine(graphics, x, yOff + 5 , x + 230, yOff + 5, ColorRed.rgba);
                } else if (NULL == disassembly->map || NULL == disassembly->map[i]) {
                        GraphicsDrawLine(graphics, x, yOff + 5 , x + 230, yOff + 5, ColorGray.rgba);
                } else if (i == pc) {
                        GraphicsDrawText(graphics, x, yOff, disassembly->map[i]->text, FONT_SCALE, ColorBlue.rgba);
                } else {
                        GraphicsDrawText(graphics, x, yOff, disassembly->map[i]->text, FONT_SCALE, ColorBlack.rgba);
                }
        }
}

int main(int argc, char **argv) {
        Init();

        CpuConnectBus(cpu, bus);
        BusAttachCart(bus, cart);

        BusReset(bus);

        // Disassemble
        struct disassembly *disassembly = DisassemblyInit(cpu, 0x0010, 0x0030);

        struct timespec frameEnd;
        struct timespec frameStart;
        clock_gettime(CLOCK_REALTIME, &frameStart);

        double residualTime = 0.0;
        int isEmulating = 1;
        int isRunning = 1;
        int selectedPalette = 0;
        int soundPlaying = 0;
        while (isRunning) {
                clock_gettime(CLOCK_REALTIME, &frameEnd);
                double elapsedTime = S_AS_MS(frameEnd.tv_sec - frameStart.tv_sec);
                elapsedTime += NS_AS_MS(frameEnd.tv_nsec - frameStart.tv_nsec);

                GraphicsBegin(graphics);
                GraphicsClearScreen(graphics, 0xFFFFFFFF);

                InputProcess(input);
                isRunning = !InputIsQuitRequested(input);

                struct controller *controllers = BusGetControllers(bus);
                controllers[0].input = 0x00;
                controllers[0].input |= InputGetKey(input, KEY_X).held ? 0x80 : 0x00;
                controllers[0].input |= InputGetKey(input, KEY_Z).held ? 0x40 : 0x00;
                controllers[0].input |= InputGetKey(input, KEY_A).held ? 0x20 : 0x00;
                controllers[0].input |= InputGetKey(input, KEY_S).held ? 0x10 : 0x00;
                controllers[0].input |= InputGetKey(input, KEY_UP).held ? 0x08 : 0x00;
                controllers[0].input |= InputGetKey(input, KEY_DOWN).held ? 0x04 : 0x00;
                controllers[0].input |= InputGetKey(input, KEY_LEFT).held ? 0x02 : 0x00;
                controllers[0].input |= InputGetKey(input, KEY_RIGHT).held ? 0x01 : 0x00;

                if (InputGetKey(input, KEY_SPACE).pressed) isEmulating = !isEmulating;
                if (InputGetKey(input, KEY_R).pressed) BusReset(bus);
                if (InputGetKey(input, KEY_P).pressed) {
                        ++selectedPalette;
                        selectedPalette &= 0x07;
                }
                if (InputGetKey(input, KEY_M).pressed) {
                        soundPlaying = !soundPlaying;
                        if (soundPlaying) {
                                SoundStop();
                        } else {
                                SoundPlay();
                        }
                }

                if (isEmulating) {
                        if (0.0f <= residualTime) {
                                residualTime -= elapsedTime;
                        } else {
                                residualTime += (1.0 / 60.0) - elapsedTime;
                                do { BusTick(bus); } while (!PpuIsFrameComplete(ppu));
                                PpuResetFrameCompletion(ppu);
                        }
                } else {
                        // Emulate code step-by-step.
                        if (InputGetKey(input, KEY_C).pressed) {
                                // Tick enough times to execute a whole CPU instruction.
                                do { BusTick(bus); } while (!CpuIsComplete(cpu));

                                // The CPU clock runs slower than system clock,
                                // so it may be incomplete for additional system
                                // clock cycles. Drain those out.
                                do { BusTick(bus); } while (CpuIsComplete(cpu));
                        }

                        // Emulate one whole frame.
                        if (InputGetKey(input, KEY_F).pressed) {
                                // Clock enough times to draw a single frame.
                                do { BusTick(bus); } while (!PpuIsFrameComplete(ppu));

                                // Use residual clock cycles to complete the
                                // current instruction.
                                do { BusTick(bus); } while (!CpuIsComplete(cpu));

                                // Reset frame completion flag.
                                PpuResetFrameCompletion(ppu);
                        }
                }

                GraphicsDrawLine(graphics, NES_SCREEN_WIDTH, 0, NES_SCREEN_WIDTH, HEIGHT, ColorBlack.rgba);
                DrawCpuState(NES_SCREEN_WIDTH + 10, HEIGHT - (FONT_HEADER_SCALE + 5));

                GraphicsDrawLine(graphics, NES_SCREEN_WIDTH, HEIGHT - 160, WIDTH, HEIGHT - 160, ColorBlack.rgba);
                DrawDisassembly(disassembly, NES_SCREEN_WIDTH + 10, HEIGHT - 160 - (FONT_HEADER_SCALE + 5), 20);

                // Iterate through each palette.
                for (int p = 0; p < 8; p++)
                        for (int s = 0; s < 4; s++) {
                                struct color *color = PpuGetColorFromPaletteRam(ppu, p, s);
                                int x = NES_SCREEN_WIDTH + 1 + (p * 5 * (SWATCH_SIZE + 1)) + (s * (SWATCH_SIZE + 1));
                                int y = NES_SCREEN_HEIGHT - 590;
                                GraphicsDrawFilledRect(graphics, x, y, SWATCH_SIZE, SWATCH_SIZE, color->rgba);
                        }

                // Draw selection reticule around selected palette.
                GraphicsDrawRect(graphics, NES_SCREEN_WIDTH + (selectedPalette * 5 * (SWATCH_SIZE + 1)), NES_SCREEN_HEIGHT - 591, SWATCH_SIZE * 4 + 4, SWATCH_SIZE + 1, ColorBlack.rgba);

                // Draw the pattern tables.
                GraphicsDrawSprite(graphics, NES_SCREEN_WIDTH, HEIGHT - 719, PpuGetPatternTable(ppu, 0, selectedPalette), 1);
                GraphicsDrawSprite(graphics, NES_SCREEN_WIDTH + 129, HEIGHT - 719, PpuGetPatternTable(ppu, 1, selectedPalette), 1);

                GraphicsDrawSprite(graphics, 0, 0, PpuScreen(ppu), 3);

                GraphicsEnd(graphics);
        }

        DisassemblyDeinit(disassembly);

        // Reset
        BusReset(bus);

        Deinit(0);
        return 0;
}

float SquareWave(float frequency, float seconds) {
        static float numHarmonics = 20;
        static float dutyCycle = 0.5f;

        float a = 0;
        float b = 0;
        float phaseDiff = dutyCycle * 2.0f * 3.14159f;

        for (float n = 1; n < numHarmonics; n++) {
                float c = n * frequency * 2.0 * 3.14159f * seconds;
                a += sin(c) / n;
                b += sin(c - phaseDiff * n) / n;
        }

        return (2.0 / 3.14159f) * (a - b);
}

float SynthFn(int channel, float timeElapsedS, float timeStepS) {
        static float frequency = 440.0f;

        //return sin(timeElapsedS * 440.0f * 2.0f * 3.14159f);
        return 0.5f * SquareWave(frequency, timeElapsedS);
}
