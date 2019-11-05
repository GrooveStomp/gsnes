/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: sprite.c
  Created: 2019-11-05
  Updated: 2019-11-05
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
******************************************************************************/
#include <stdlib.h> // malloc, free
#include <math.h> // fmin

#include "sprite.h"
#include "color.h"
//! \file sprite.c

struct sprite {
        struct color *pixels;
        uint32_t width;
        uint32_t height;
};

struct sprite *SpriteInit(unsigned int width, unsigned int height) {
        struct sprite *sprite = (struct sprite *)malloc(sizeof(struct sprite));
        if (NULL == sprite) {
                return NULL;
        }

        sprite->pixels = (struct color *)malloc(sizeof(struct color) * width * height);
        if (NULL == sprite->pixels) {
                free(sprite);
                return NULL;
        }

        sprite->width = width;
        sprite->height = height;

        return sprite;
}

void SpriteDeinit(struct sprite *sprite) {
        if (NULL == sprite) {
                return;
        }

        if (NULL != sprite->pixels) {
                free(sprite->pixels);
                sprite->pixels = NULL;
        }

        free(sprite);
}

void SpriteSetPixel(struct sprite *sprite, unsigned int x, unsigned int y, uint32_t rgba) {
        if (x >= 0 && x < sprite->width && y >= 0 && y < sprite->height) {
                sprite->pixels[y * sprite->width + x] = ColorInitInt(rgba);
        }
}

uint32_t SpriteSample(struct sprite *sprite, float x, float y) {
        int32_t sx = fminf(x * (float)sprite->width, sprite->width - 1);
        int32_t sy = fminf(y * (float)sprite->height, sprite->height - 1);

        if (sx >= 0 && sx < sprite->width && sy >= 0 && sy < sprite->height) {
                return sprite->pixels[sy * sprite->width + sx].rgba;
        }

        return ColorInitFloats(0, 0, 0, 0).rgba;
}

struct color *SpriteData(struct sprite *sprite) {
        return sprite->pixels;
}
