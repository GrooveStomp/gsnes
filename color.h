/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: color.h
  Created: 2019-08-15
  Updated: 2019-11-05
  Author: Aaron Oman
  Notice: GNU GPLv3 License

  Based off of: One Lone Coder Console Game Engine Copyright (C) 2018 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
#include <stdint.h>

//! \file color.h
//! This interface attempts to provide an intuitive wrapper around "raw"
//! unsigned integer colors.
//!
//! An unsigned integer color is packed 32-bit value consisting of 4 pixel
//! elements: RGBA.  These elements are stored as written: RGBA, or, visually
//! mapped as hex symbols: RRGGBBAA.

#ifndef COLOR_VERSION
#define COLOR_VERSION "0.2-gsnes" //!< include guard

//! RGBA color quad
struct color {
        uint32_t rgba;
};

//! \brief Initialize a new color with individual R, G, B, A components as floats.
//!
//! \param r Red component from 0 to 1
//! \param g Green componenet from 0 to 1
//! \param b Blue component from 0 to 1
//! \param a Alpha component, from 0 to 1
//! \return resulting color object
struct color
ColorInitFloats(float r, float g, float b, float a);

//! \brief Initialize a new color with individual R, G, B, A components as ints.
//!
//! \param r Red component from 0 to 255
//! \param g Green componenet from 0 to 255
//! \param b Blue component from 0 to 255
//! \param a Alpha component, from 0 to 255
//! \return resulting color object
struct color
ColorInitInts(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

//! \brief Initialize a new color with an rgba component from a color
//!
//! \param rgba 32-bit r,g,b,a packed int.
//! \return resulting color object
struct color
ColorInitInt(uint32_t rgba);

//! \brief Get the color component
//!
//! The component is returned as the raw integer value, in the range [0,255]
//!
//! \param color color object to read
//! \param component 'r', 'g', 'b' or 'a' exclusively.
//! \return value of the color component
unsigned int
ColorGetInt(struct color color, char component);

//! \brief Get the color component
//!
//! The component is returned as a float in the range [0.0,1.0]
//!
//! \param color color object to read
//! \param component 'r', 'g', 'b' or 'a' exclusively.
//! \return value of the color component
float
ColorGetFloat(struct color color, char component);

//! \brief Set the color component
//!
//! The value should be an integer in the range [0,255]
//!
//! \param color pointer to the color object to write
//! \param component 'r', 'g', 'b' or 'a' exclusively.
//! \param value value of the color component to set
void
ColorSetInt(struct color *color, char component, unsigned int value);

//! \brief Set the color component
//!
//! The value should be a float in the range [0.0,1.0]
//!
//! \param color pointer to the color object to write
//! \param component 'r', 'g', 'b' or 'a' exclusively.
//! \param value value of the color component to set
void
ColorSetFloat(struct color *color, char component, float value);

extern struct color ColorWhite;
extern struct color ColorBlack;
extern struct color ColorRed;
extern struct color ColorGreen;
extern struct color ColorBlue;
extern struct color ColorPurple;
extern struct color ColorYellow;
extern struct color ColorCyan;
extern struct color ColorPink;

#endif // COLOR_VERSION
