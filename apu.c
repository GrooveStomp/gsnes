/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: apu.c
  Created: 2019-12-17
  Updated: 2019-12-17
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file apu.c
#include <stdbool.h>
#include <stdlib.h>

#include "apu.h"
#include "util.h"

struct sequencer {
        uint32_t sequence;
        uint16_t timer;
        uint16_t reload;
        uint8_t output;
};

uint8_t SeqTick(struct sequencer *s, bool isEnabled, void (*fn)(uint32_t *)) {
        if (!isEnabled) return s->output;

        s->timer--;
        if (s->timer == 0xFFFF) { // -1
                s->timer = s->reload + 1;
                fn(&(s->sequence));
                s->output = s->sequence & 0x00000001;
        }

        return s->output;
}

struct apu {
        struct sequencer pulse1Seq;
        bool pulse1Enable;
        double pulse1Sample;
        bool isSampleAvailable;
        uint32_t frameTickCount;
        uint32_t tickCount;

        bool didLastTickProduceSample;
};

struct apu *ApuInit() {
        struct apu *apu = (struct apu *)calloc(1, sizeof(struct apu));
        if (apu == NULL) {
                return NULL;
        }

        apu->pulse1Enable = false;
        apu->pulse1Sample = 0.0;
        apu->isSampleAvailable = false;
        apu->frameTickCount = 0;
        apu->tickCount = 0;
        apu->didLastTickProduceSample = false;

        return apu;
}

void ApuDeinit(struct apu *apu) {
}

void ApuWrite(struct apu *apu, uint16_t addr, uint8_t data) {
        switch (addr) {
                case 0x4000:
                        switch ((data & 0xC) >> 6) {
                                case 0x00:
                                        apu->pulse1Seq.sequence = 0x0001; // 1/8: 00000001
                                        break;
                                case 0x01:
                                        apu->pulse1Seq.sequence = 0x0003; // 1/4: 00000011
                                        break;
                                case 0x02:
                                        apu->pulse1Seq.sequence = 0x000F; // 1/2: 00001111
                                        break;
                                case 0x03:
                                        apu->pulse1Seq.sequence = 0x00F4; // 3/4: 11111100
                                        break;
                        }
                        break;

                case 0x4001:
                        break;

                case 0x4002:
                        apu->pulse1Seq.reload = (uint16_t)((data & 0x07)) << 8 | (apu->pulse1Seq.reload & 0x00FF);
                        apu->pulse1Seq.timer = apu->pulse1Seq.reload;
                        break;

                case 0x4003:
                        break;

                case 0x4004:
                        break;

                case 0x4005:
                        break;

                case 0x4006:
                        break;

                case 0x4007:
                        break;

                case 0x4008:
                        break;

                case 0x400C:
                        break;

                case 0x400E:
                        break;

                case 0x4015:
                        apu->pulse1Enable = data & 0x01;
                        break;

                case 0x400F:
                        break;
        }
}

uint8_t ApuRead(struct apu *apu, uint16_t addr) {
        return 0x00;
}

void Pulse1Sequence(uint32_t *sequence) {
        uint32_t in = *sequence;
        uint8_t rotated = RotateRightByte((uint8_t)in);
        *sequence = (uint32_t)rotated;
}

void ApuTick(struct apu *apu) {
        bool isFrameQuarterTick = false;
        bool isFrameHalfTick = false;
        apu->didLastTickProduceSample = false;

        if (apu->tickCount % 6 == 0) {
                apu->frameTickCount++;

                // 4-Step sequence mode.
                if (apu->frameTickCount == 3729) {
                        isFrameQuarterTick = true;
                }

                if (apu->frameTickCount == 7457) {
                        isFrameQuarterTick = true;
                        isFrameHalfTick = true;
                }

                if (apu->frameTickCount == 11186) {
                        isFrameQuarterTick = true;
                }

                if (apu->frameTickCount == 14916) {
                        isFrameQuarterTick = true;
                        isFrameHalfTick = true;
                        apu->frameTickCount = 0;
                }

                // Quarter frame beats adjust the volume envelope.
                if (isFrameQuarterTick) {
                }

                // Half frame beats adjust the note length and frequency sweeps.
                if (isFrameHalfTick) {
                }

                SeqTick(&apu->pulse1Seq, apu->pulse1Enable, Pulse1Sequence);
                apu->didLastTickProduceSample = true;
        }

        apu->tickCount++;
}

void ApuReset(struct apu *apu) {
}

double ApuGetOutputSample(struct apu *apu) {
        return apu->pulse1Sample;
}

uint8_t ApuDidLastTickProduceSample(struct apu *apu) {
        return apu->didLastTickProduceSample;
}
