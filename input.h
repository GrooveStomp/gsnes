/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: input.h
  Created: 2019-07-21
  Updated: 2019-11-09
  Author: Aaron Oman
  Notice: GNU GPLv3 License

  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file input.h
//! A small interface to manage handling input separately from main().

#ifndef INPUT_VERSION
#define INPUT_VERSION "0.2-gsnes" //!< include guard and version info

struct input;

//! \brief Creates and initializes new input state
//! \return The initialized input state
struct input *
InputInit();

//! \brief De-initializes and frees memory for the input state
//! \param[in,out] input
void
InputDeinit(struct input *input);

//! \brief Handle program inputs
//!
//! Processes all program inputs and stores relevant data internally in input.
//!
//! \param[in,out] input
void
InputProcess(struct input *input);

//! \brief Check if the user has tried to quit the program
//!
//! \param[in,out] input
//! \return 1 if quit has been pressed, otherwise 0
int
InputIsQuitRequested(struct input *input);

enum input_key_enum {
        KEY_A = 0,
        KEY_B,
        KEY_C,
        KEY_D,
        KEY_E,
        KEY_F,
        KEY_G,
        KEY_H,
        KEY_I,
        KEY_J,
        KEY_K,
        KEY_L,
        KEY_M,
        KEY_N,
        KEY_O,
        KEY_P,
        KEY_Q,
        KEY_R,
        KEY_S,
        KEY_T,
        KEY_U,
        KEY_V,
        KEY_W,
        KEY_X,
        KEY_Y,
        KEY_Z,
        KEY_LEFT,
        KEY_RIGHT,
        KEY_UP,
        KEY_DOWN,
        KEY_ENTER,
        KEY_ESC,
        KEY_LSHIFT,
        KEY_RSHIFT,
        KEY_LCTRL,
        KEY_RCTRL,
        KEY_SPACE,
        KEY_1,
        KEY_2,
        KEY_3,
        KEY_4,
        KEY_5,
        KEY_6,
        KEY_7,
        KEY_8,
        KEY_9,
        KEY_0
};

unsigned int
InputIsKeyPressed(struct input *input, enum input_key_enum e);

#endif // INPUT_VERSION
