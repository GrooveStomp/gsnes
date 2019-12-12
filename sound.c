/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: sound.c
  Created: 2019-12-12
  Updated: 2019-12-12
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
#include <stdbool.h>
#include <stdatomic.h>

// TODO: Partially done switching from libsoundio to alsa. See: https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Extensions/olcPGEX_Sound.h
// TODO: Next is GetMixerOutput

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#pragma pack(push, 1)
struct wave_format {
        uint16_t formatTag;
        uint16_t channels;
        uint32_t samplesPerSec;
        uint32_t avgBytesPerSec;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
        uint16_t size;
};
#pragma pack(pop)

struct sample {
        float *samples;
        long samplesCount;
        int channels;
        bool isValid;
};

struct sample SampleInit() {
        struct sample sample;
        sample.samples = NULL;
        sample.samplesCount = 0;
        sample.channels = 0;
        sample.isValid = false;
        return sample;
}

struct active_sample {
        int sampleId;
        uint64_t samplePosition;
        bool isFinished;
        bool shouldLoop;
        bool shouldStop;
};

struct active_sample ActiveSampleInit() {
        struct active_sample activeSample;
        activeSample.sampleId = 0;
        activeSample.samplePosition = 0;
        activeSample.isFinished = false;
        activeSample.shouldLoop = false;
        activeSample.shouldStop = false;
        return activeSample;
}

typedef float (*synth_fn)(int, float, float);
typedef float (*filter_fn)(int, float, float);

//! Sound state
struct sound {
        struct active_sample *activeSamples;
        int activeSamplesCount;
        int activeSamplesCapacity;
        snd_pcm_t *pcm;
        unsigned int sampleRate;
        unsigned int channels;
        unsigned int blockSamples;
        unsigned short blockMemory;
        pthread_t thread;
        atomic_bool isThreadActive;
        atomic_uint_fast64_t globalElapsedTimeNs;
        synth_fn synthFn;
        filter_fn filterFn;
};

void SoundDeinit(struct sound *sound) {
        if (NULL == sound)
                return;

        atomic_store(sound->isThreadActive, false);

        int exitStatus = 0; // See call to pthread_exit in thread.
        int rc = pthread_join(sound->thread, &exitStatus);
        // Possible rc values:
        // EDEADLK: Deadlock.
        // EINVAL: Not a joinable thread.
        // EINVAL: Another thread is waiting for this thread to join.
        // ESRCH: Thread ID not found.

        snd_pcm_drain(sound->pcm);
        snd_pcm_close(sound->pcm);

        if (NULL != sound->activeSamples)
                free(sound->activeSamples);

        free(sound);
}

float Clip(float sample, float max) {
        if (sample >= 0.0)
                return fminf(sample, max);
        else
                return fmaxf(sample, -max);
}

static void AudioThreadFunc(*void args) {
        struct sound *sound = (struct sound *)args;
        atomic_store(sound->globalElapsedTimeNs, 0);

        static float timeStep = 1.0f / (float)sound->sampleRate;

        // Goofy hack to get maximum integer for a type at run-time
        short maxSampleShort = (short)pow(2, (sizeof(short) * 8) - 1) - 1;
        float maxSample = (float)maxSampleShort;

        short previousSample = 0;
        while (atomic_load(struct->isThreadActive)) {
                short newSample = 0;

                for (unsigned int n = 0; n < sound->blockSamples; n += sound->channels) {
                        for (unsigned int c = 0; c < sound->channels; c++) {
                                uint64_t elapsedNs = atomic_load(sound->globalElapsedTimeNs);
                                float output = GetMixerOutput(c, NS_AS_S(elapsedNs), timeStep);
                                newSample = (short)(Clip(output, 1.0) * maxSample);
                                sound->blockMemory[n + c] = newSample;
                                previousSample = newSample;
                        }
                        uint64_t stepNs = (uint64_t)S_AS_NS(timeStep);
                        uint64_t elapsedNs = atomic_load(sound->globalElapsedTimeNs);
                        atomic_store(sound->globalElapsedTimeNs, elapsedNs + stepNs);
                }

                snd_pcm_uframes_t remaining = sound->blockSamples;
                short *blockPos = sound->blockMemory;
                while (remaining > 0) {
                        int rc = snd_pcm_writei(sound->pcm, blockPos, remaining);
                        if (rc > 0) {
                                blockPos += rc * sound->channels;
                                remaining -= rc;
                        }
                        if (rc == -EAGAIN) continue;

                        // buffer underrun, get more data
                        if (rc == -EPIPE) {
                                snd_pcm_prepare(sound->pcm);
                        }
                }
        }

        int retval = 0;
        pthread_exit(retval);
}

struct sound *SoundInit(unsigned int sampleRate, unsigned int channels, unsigned int blocks, unsigned int blockSamples) {
        struct sound *sound = (struct sound *)calloc(1, sizeof(struct sound));
        if (sound == NULL) {
                fprintf(stderr, "Couldn't allocate sound struct\n");
                return NULL;
        }

        atomic_store(sound->isThreadActive, false);
        sound->sampleRate = sampleRate;
        sound->channels = channels;
        sound->blockSamples = blockSamples;
        sound->blockMemory = NULL;
        sound->synthFn = NULL;
        sound->filterFn = NULL;

        int rc = snd_pcm_open(sound->pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
        if (rc < 0) {
                fprintf(stderr, "Couldn't initialize pcm\n");
                SoundDeinit(sound);
                return NULL;
        }

        snd_pcm_hw_params_t *params;
        snd_pcm_hw_params_alloca(&params);
        snd_pcm_hw_params_any(sound->pcm, params);

        snd_pcm_hw_params_set_format(sound->pcm, params, SND_PCM_FORMAT_S16_LE);
        snd_pcm_hw_params_set_rate(sound->pcm, params, sound->sampleRate, 0);
        snd_pcm_hw_params_set_channels(sound->pcm, params, sound->channels);
        snd_pcm_hw_params_set_period_size(sound->pcm, params, sound->blockSamples, 0);
        snd_pcm_hw_params_set_periods(sound->pcm, params, blocks, 0);

        rc = snd_pcm_hw_params(sound->pcm, params);
        if (rc < 0) {
                fprintf(stderr, "Couldn't configure pcm device\n");
                SoundDeinit(sound);
                return NULL;
        }

        sound->activeSamples = calloc(1, sizeof(struct active_sample));
        if (sound->activeSamples == NULL) {
                fprintf(stderr, "Couldn't allocate active samples memory\n");
                SoundDeinit(sound);
                return NULL;
        }
        sound->activeSamplesCount = 0;
        sound->activeSamplesCapacity = 1;

        sound->blockMemory = (short *)calloc(sound->blockSamples, sizeof(short));
        if (sound->blockMemory == NULL) {
                fprintf(stderr, "Couldn't allocate block memory\n");
                SoundDeinit(sound);
                return NULL;
        }

        snd_pcm_start(sound->pcm);
        for (unsigned int i = 0; i < blocks; i++) {
                rc = snd_pcm_writei(sound->pcm, sound->blockMemory, 512);
                // TODO: What about result rc there?
        }

        snd_pcm_start(sound->pcm);
        atomic_store(sound->isThreadActive, true);

        int err;
        if (0 != (err = pthread_create(sound->thread, NULL, AudioThreadFunc, &sound))) {
                fprintf(stderr, "Couldn't create sound thread: errno(%d)\n", err);
                SoundDeinit(sound);
                return NULL;
        }

        return sound;
}
