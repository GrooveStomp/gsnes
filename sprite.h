/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: sprite.h
  Created: 2019-11-05
  Updated: 2019-11-05
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
#include <stdint.h>
//! \file sprite.h

struct sprite;
struct color;

struct sprite *
SpriteInit(unsigned int width, unsigned int height);

void
SpriteDeinit(struct sprite *sprite);

void
SpriteSetPixel(struct sprite *sprite, unsigned int x, unsigned int y, uint32_t rgba);

uint32_t
SpriteSample(struct sprite *sprite, float x, float y);

struct color *
SpriteData(struct sprite *sprite);
