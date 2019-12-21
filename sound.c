/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: sound.c
  Created: 2019-07-07
  Updated: 2019-12-16
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file sound.c
#include <math.h> // fmod, M_PI
#include <string.h> // memset
#include <stdio.h> // fprintf
#include <stdlib.h> //malloc, free
#include <stdint.h>

#include <soundio/soundio.h>

#include "sound.h"
#include "util.h"

//! Sound state
struct sound {
        struct SoundIo *lib;
        struct SoundIoDevice *dev;
        struct SoundIoOutStream *stream;

        sound_synth_fn synthFn;
        sound_filter_fn filterFn;
};

static struct sound *sound;

void SoundSetSynthFn(sound_synth_fn synthFn) {
        sound->synthFn = synthFn;
}

void SoundSetFilterFn(sound_filter_fn filterFn) {
        sound->filterFn = filterFn;
}

void SoundStop() {
        int err;
        if ((err = soundio_outstream_pause(sound->stream, 1))) {
                fprintf(stderr, "Unable to stop device: %s\n", soundio_strerror(err));
        }
}

void SoundDeinit() {
        if (NULL == sound)
                return;

        if (NULL != sound)
                SoundStop(sound);

        if (NULL != sound->stream)
                soundio_outstream_destroy(sound->stream);

        if (NULL != sound->dev)
                soundio_device_unref(sound->dev);

        if (NULL != sound->lib)
                soundio_destroy(sound->lib);

        free(sound);
}

static void WriteCallback(struct SoundIoOutStream *out, int frameCountMin, int frameCountMax) {
        const struct SoundIoChannelLayout *layout = &out->layout;
        float sampleRate = out->sample_rate;
        float secondsPerFrame = 1.0f / sampleRate;
        struct SoundIoChannelArea *areas;
        int framesLeft = frameCountMax;
        int err;

        static float secondsOffset = 0.0f;

        while (framesLeft > 0) {
                int frameCount = framesLeft;

                if ((err = soundio_outstream_begin_write(out, &areas, &frameCount))) {
                        fprintf(stderr, "%s\n", soundio_strerror(err));
                        fprintf(stderr, "TODO: AARON: Improve exit here");
                        exit(1);
                }

                if (!frameCount)
                        break;

                for (int frame = 0; frame < frameCount; frame++) {
                        for (int channel = 0; channel < layout->channel_count; channel++) {
                                float *ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);

                                float secondsElapsed = secondsOffset + ((float)frame * secondsPerFrame);
                                float *sample = NULL;
                                if (sound->synthFn != NULL) {
                                        sample = sound->synthFn(channel, secondsElapsed, secondsPerFrame);
                                }
                                if (sound->filterFn != NULL) {
                                        *sample = sound->filterFn(channel, secondsElapsed, *sample);
                                }

                                if (sample == NULL) {
                                        *ptr = 0;
                                } else {
                                        *ptr = *sample;
                                }
                        }
                }
                secondsOffset = fmod(secondsOffset + secondsPerFrame * frameCount, 1.0);

                if ((err = soundio_outstream_end_write(out))) {
                        fprintf(stderr, "%s\n", soundio_strerror(err));
                        exit(1);
                }

                framesLeft -= frameCount;
        }
}

void SoundPlay() {
        int err;
        if ((err = soundio_outstream_pause(sound->stream, 0))) {
                fprintf(stderr, "Unable to start device: %s\n", soundio_strerror(err));
        }
}

struct sound *SoundInit(unsigned int sampleRate, int numChannels) {
        int err;

        sound = (struct sound *)malloc(sizeof(struct sound));
        memset(sound, 0, sizeof(struct sound));

        sound->lib = soundio_create();
        if (!sound->lib) {
                fprintf(stderr, "Couldn't initialize soundio\n");
                return NULL;
        }

        if ((err = soundio_connect(sound->lib))) {
                fprintf(stderr, "Error connecting to soundio: %s\n", soundio_strerror(err));
                SoundDeinit();
                return NULL;
        }
        soundio_flush_events(sound->lib);

        int defaultOutDevIndex = soundio_default_output_device_index(sound->lib);
        if (defaultOutDevIndex < 0) {
                fprintf(stderr, "No output device found\n");
                SoundDeinit();
                return NULL;
        }

        sound->dev = soundio_get_output_device(sound->lib, defaultOutDevIndex);
        if (!sound->dev) {
                fprintf(stderr, "Out of memory\n");
                SoundDeinit();
                return NULL;
        }

        sound->stream = soundio_outstream_create(sound->dev);
        if (!sound->stream) {
                fprintf(stderr, "Couldn't create outstream device\n");
                SoundDeinit();
                return NULL;
        }
        sound->stream->format = SoundIoFormatFloat32NE;
        sound->stream->sample_rate = sampleRate;
        sound->stream->write_callback = WriteCallback;

        if ((err = soundio_outstream_open(sound->stream))) {
                fprintf(stderr, "Unable to open device: %s", soundio_strerror(err));
                SoundDeinit();
                return NULL;
        }

        if (sound->stream->layout_error)
                fprintf(stderr, "Unable to set channel layout: %s\n", soundio_strerror(err));

        if ((err = soundio_outstream_start(sound->stream))) {
                fprintf(stderr, "Unable to start device: %s\n", soundio_strerror(err));
                SoundDeinit();
                return NULL;
        }

        SoundStop(sound);

        return sound;
}
