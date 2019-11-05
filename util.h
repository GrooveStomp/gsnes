/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: util.h
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
//! \file util.h

#ifndef UTIL_VERSION
#define UTIL_VERSION "0.1.0"

#define S_AS_MS(x) (x) * 1000.0 //!< Convert seconds to milliseconds
#define NS_AS_MS(x) (x) / 1000000.0 //!< Convert nanoseconds to milliseconds
#define MS_AS_NS(x) (x) * 1000000.0 //!< Convert milliseconds to nanoseconds
#define HZ_AS_MS(x) (1.0 / (x)) * 1000.0 //!< Convert hertz to milliseconds per frame

#define B_AS_KB(x) (x) * (1.0 / 1024)
#define B_AS_MB(x) B_TO_KB((x)) * (1.0 / 1024)
#define B_AS_GB(x) B_TO_MB((x)) * (1.0 / 1024)

#define KB_AS_B(x) (x) * 1024
#define MB_AS_B(x) KB((x)) * 1024
#define GB_AS_B(x) MB((x)) * 1024

#endif // UTIL_VERSION
