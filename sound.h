/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: sound.h
  Created: 2019-07-07
  Updated: 2019-12-16
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file sound.h
//!
//! A very small interface to sound playback.
//!
//! Extremely limited in scope; currently only supports playing back a 440hz
//! tone, aka A4.
#ifndef SOUND_VERSION
#define SOUND_VERSION "0.1.0-gsnes"

typedef float (*sound_synth_fn)(int channel, float timeElapsedS, float timeStepS);
typedef float (*sound_filter_fn)(int channel, float timeElapsedS, float sample);

struct sound;

//! \brief Creates and initializes a new sound object
//! \return The initialized sound object
struct sound *
SoundInit(unsigned int sampleRate, int numChannels);

//! \brief De-initializes and frees memory for the given sound object
//! \param[in,out] sound The initialized sound object to be cleaned and reclaimed
void
SoundDeinit();

//! \brief Start playback of A4 (440hz)
//! \param[in,out] sound Sound interface to invoke playback on
void
SoundPlay();

//! \brief Stop playback
//! \param[in,out] sound Sound interface to stop playback on
void
SoundStop();

void
SoundSetSynthFn(sound_synth_fn synthFn);

void
SoundSetSynthFn(sound_filter_fn filterFn);

#endif // SOUND_VERSION
