/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: util.h
  Created: 2019-11-03
  Updated: 2019-12-07
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file util.h
#include <stdint.h>

#ifndef UTIL_VERSION
#define UTIL_VERSION "0.1-gsnes"

#define S_AS_MS(x) (x) * 1000.0 //!< Convert seconds to milliseconds
#define NS_AS_MS(x) (x) / 1000000.0 //!< Convert nanoseconds to milliseconds
#define MS_AS_NS(x) (x) * 1000000.0 //!< Convert milliseconds to nanoseconds
#define S_AS_NS(x) (x) * 1000000000 //!< Convert seconds to nanoseconds
#define HZ_AS_MS(x) (1.0 / (x)) * 1000.0 //!< Convert hertz to milliseconds per frame

#define B_AS_KB(x) (x) * (1.0 / 1024)
#define B_AS_MB(x) B_TO_KB((x)) * (1.0 / 1024)
#define B_AS_GB(x) B_TO_MB((x)) * (1.0 / 1024)

#define KB_AS_B(x) (x) * 1024
#define MB_AS_B(x) KB((x)) * 1024
#define GB_AS_B(x) MB((x)) * 1024

//! \brief A mostly generic implementation of swap
//!
//! Both v1 and v2 must point to data that is the same size, as specified in the
//! size parameter.
//!
//! \param[in,out] v1 first value
//! \param[in,out] v2 second value
//! \param[in] size v1 and v2 must each be this size
void
SwapGeneric(void *v1, void *v2, size_t size);

//! \brief convert number to hexadecimal string
//!
//! \param[in] hex number to stringify
//! \param[in] nibbles number of nibbles in data source
//! \param[in,out] buf pre-allocated char buffer
//! \param[in] size size of buf
void
HexToString(uint32_t hex, uint8_t nibbles, char *buf, uint8_t size);

//! \brief Mirrors bits left <-> right in a byte.
//!
//! eg.: 1100_0001 => 1000_0011
//!
//! \param[in] b byte to flip
//! \return flipped byte
uint8_t
MirrorByte(uint8_t b);

#endif // UTIL_VERSION
