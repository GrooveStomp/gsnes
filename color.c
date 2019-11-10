/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: color.c
  Created: 2019-08-15
  Updated: 2019-11-05
  Author: Aaron Oman
  Notice: GNU GPLv3 License

  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file color.c

#include "color.h"

struct color ColorWhite = { 0xFFFFFFFF };
struct color ColorBlack = { 0x000000FF };
struct color ColorRed = { 0xFF0000FF };
struct color ColorGreen = { 0x00FF00FF };
struct color ColorBlue = { 0x0000FFFF };
struct color ColorPurple = { 0x7F00FFFF };
struct color ColorYellow = { 0xFFFF00FF };
struct color ColorCyan = { 0x00FFFFFF };
struct color ColorPink = { 0xFF00FFFF };

void ColorSetInt(struct color *color, char component, unsigned int value) {
        unsigned int pos = 0;
        switch (component) {
                case 'r':
                        pos = 3;
                        break;
                case 'g':
                        pos = 2;
                        break;
                case 'b':
                        pos = 1;
                        break;
                case 'a':
                        pos = 0;
                        break;
                default:
                        pos = 0;
        }

        unsigned int shift = pos * 8;

        unsigned int rgba = color->rgba & ~(0xFF << shift);
        color->rgba = rgba | (value << shift);
}

void ColorSetFloat(struct color *color, char component, float value) {
        unsigned int intVal = (unsigned int)(value * 255.0f);
        ColorSetInt(color, component, intVal);
}

struct color ColorInitFloats(float r, float g, float b, float a) {
        struct color color = { 0 };

        ColorSetFloat(&color, 'r', r);
        ColorSetFloat(&color, 'g', g);
        ColorSetFloat(&color, 'b', b);
        ColorSetFloat(&color, 'a', a);

        return color;
}

unsigned int ColorGetInt(struct color color, char component) {
        unsigned int pos = 0;
        switch (component) {
                case 'r':
                        pos = 3;
                        break;
                case 'g':
                        pos = 2;
                        break;
                case 'b':
                        pos = 1;
                        break;
                case 'a':
                        pos = 0;
                        break;
                default:
                        pos = 0;
        }

        unsigned int shift = pos * 8;
        return (color.rgba >> shift) & 0xFF;
}

float ColorGetFloat(struct color color, char component) {
        int value = ColorGetInt(color, component);
        return (float)value / 255.0f;
}

struct color ColorInitInts(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        struct color color = { 0 };

        ColorSetInt(&color, 'r', r);
        ColorSetInt(&color, 'g', g);
        ColorSetInt(&color, 'b', b);
        ColorSetInt(&color, 'a', a);

        return color;
}

struct color ColorInitInt(uint32_t rgba) {
        struct color color = { 0 };
        color.rgba = rgba;
        return color;
}
