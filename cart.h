/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: cart.h
  Created: 2019-11-03
  Updated: 2019-11-03
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
//! \file cart.h

#ifndef CART_VERSION
#define CART_VERSION "0.1.0"

struct cart;

struct cart *
CartInit();

void
CartDeinit(struct cart *cart);

bool
CartCpuRead(struct cart *cart, uint16_t addr, uint8_t *data);

bool
CartCpuWrite(struct cart *cart, uint16_t addr, uint8_t data);

bool
CartPpuRead(struct cart *cart, uint16_t addr, uint8_t *data);

bool
CartPpuWrite(struct cart *cart, uint16_t addr, uint8_t data);

#endif // CART_VERSION
