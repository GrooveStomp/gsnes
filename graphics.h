/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: graphics.h
  Created: 2019-07-16
  Updated: 2019-11-19
  Author: Aaron Oman
  Notice: GNU GPLv3 License

  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file graphics.h
//! Drawing interface to the operating system.
#include <stdint.h>

#ifndef GRAPHICS_VERSION
#define GRAPHICS_VERSION "0.2-gsnes" //!< include guard and version info

struct sprite;

//! \brief Creates and initializes new graphics state.
//!
//! \param[in] title The title displayed in the window titlebar
//! \param[in] width Width of the display area of the window, in pixels
//! \param[in] height Height of the display are of the window, in pixels
//! \return The initialized graphics state
struct graphics *
GraphicsInit(char *title, int width, int height);

//! \brief De-initializes and frees memory from the graphics state
//! \param[in,out] graphics
void
GraphicsDeinit(struct graphics *graphics);

//! \brief Initializes the graphics subsystem for drawing routines
//!
//! Internally locks streaming texture for direct manipulation.
//!
//! \param[in,out] graphics
void
GraphicsBegin(struct graphics *graphics);

//! \brief Prepares the graphics subsystem for presentation, then presents
//!
//! Internally unlocks streaming texture then calls presentation routines.
//!
//! \param[in,out] graphics
void
GraphicsEnd(struct graphics *graphics);

//! \brief Sets all pixels in the display buffer to the given color
//!
//! \param[in, out] graphics
//! \param[in] color 32-bit color (R|G|B|A) to clear the screen to
void
GraphicsClearScreen(struct graphics *graphics, uint32_t color);

//! \brief Put a pixel into the display buffer
//!
//! \param[in,out] graphics
//! \param[in] x display buffer x coordinate
//! \param[in] y display buffer y coordinate
//! \param[in] color 32-bit color (R|G|B|A) to set the pixel to
void
GraphicsPutPixel(struct graphics *graphics, int x, int y, uint32_t color);

//! \brief Get the color fo the pixel at (x,y) in the display buffer
//!
//! \param[in,out] graphics
//! \param[in] x display buffer x coordinate
//! \param[in] y display buffer y coordinate
//! \return 32-bit color (R|G|B|A) of the target pixel
uint32_t
GraphicsGetPixel(struct graphics *graphics, int x, int y);

//! \brief Initialize graphics state for text rendering
//!
//! \param[in,out] graphics
//! \param[in] ttfBuffer the contents of a truetype font file loaded into memory
void
GraphicsInitText(struct graphics *graphics, unsigned char *ttfBuffer);

//! \brief Draw text to the display buffer
//!
//! \param[in,out] graphics
//! \param[in] x leftmost position to start drawing text from
//! \param[in] y position from the bottom of the dispaly buffer to start drawing
//! text from
//! \param[in] string text to render
//! \param[in] fontHeight sets the height of the font in pixels
//! \param[in] color 32-bit color (R|G|B|A) to render text in
void
GraphicsDrawText(struct graphics *graphics, int x, int y, char *string, int fontHeight, uint32_t color);

//! \brief Draws a line from (x1,y1) to (x2,y2)
//!
//! \param[in,out] graphics Graphics state to be manipulated
//! \param[in] x1 horizontal position of the line start
//! \param[in] y1 vertical position of the line start
//! \param[in] x2 horizontal position of the line end
//! \param[in] y2 vertical position of the line end
//! \param[in] color 32-bit color (R|G|B|A) to render line with
void
GraphicsDrawLine(struct graphics *graphics, int x1, int y1, int x2, int y2, uint32_t color);

//! \brief Draws a sprite starting with lower left corner at (x,y)
//!
//! \param[in,out] graphics
//! \param[in] x x coordinate to start drawing from
//! \param[in] y y coordinate to start drawing from
//! \param[in] sprite sprite to draw
//! \param[in] scale integer scale of sprite
void
GraphicsDrawSprite(struct graphics *graphics, int x, int y, struct sprite *sprite, int scale);

//! \brief Draws a filled rectangle at (x,y) of width w and height h
//!
//! \param[in,out] graphics
//! \param[in] x lower left corner x coordinate
//! \param[in] y lower left corner y coordinate
//! \param[in] w width of the rectangle in pixels
//! \param[in] h height of the rectangle in pixels
//! \param[in] color 32-bit color (R|G|B|A) to render line with
void
GraphicsDrawFilledRect(struct graphics *graphics, int x, int y, int w, int h, uint32_t color);

//! \brief Draws a rectangle outline at (x,y) of width w and height h
//!
//! \param[in,out] graphics
//! \param[in] x lower left corner x coordinate
//! \param[in] y lower left corner y coordinate
//! \param[in] w width of the rectangle in pixels
//! \param[in] h height of the rectangle in pixels
//! \param[in] color 32-bit color (R|G|B|A) to render line with
void
GraphicsDrawRect(struct graphics *graphics, int x, int y, int w, int h, uint32_t color);

#endif // GRAPHICS_VERSION
