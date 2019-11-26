/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: util.c
  Created: 2019-11-15
  Updated: 2019-11-21
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder Console Game Engine Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file util.c
#include <string.h> // memmove

#include "util.h"

void SwapGeneric(void *v1, void *v2, size_t size) {
        char temp[size];
        memmove(temp, v1, size);
        memmove(v1, v2, size);
        memmove(v2, temp, size);
}

void HexToString(uint32_t hex, uint8_t nibbles, char *buf, uint8_t size) {
        if (nibbles > size) {
                nibbles = size;
        }

        for (int i = nibbles - 1; i >= 0; i--, hex >>= 4) {
                buf[i] = "0123456789ABCDEF"[hex & 0xF];
        }

        buf[nibbles] = '\0';
}
