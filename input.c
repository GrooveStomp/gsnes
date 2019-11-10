/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: input.c
  Created: 2019-06-21
  Updated: 2019-11-07
  Author: Aaron Oman
  Notice: GNU GPLv3 License

  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file input.c
#include <string.h> // memset

#include "SDL2/SDL.h"

#include "input.h"

//! \brief input state
struct input {
        const unsigned char *sdlKeyStates;
        SDL_Event event;
        int isQuitPressed;
};

struct input *InputInit() {
        struct input *input = (struct input *)malloc(sizeof(struct input));
        if (NULL == input) {
                return NULL;
        }

        memset(input, 0, sizeof(struct input));

        /* input->keyStates = (struct input_key *)calloc(KEY_MAX + 1, sizeof(struct input_key)); */
        /* if (NULL == input) { */
        /*         free(input); */
        /*         return NULL; */
        /* } */

        /* for (int i = 0; i <= KEY_MAX; i++) { */
        /*         *input->keyStates[i] = { 0 }; */
        /* } */

        input->sdlKeyStates = SDL_GetKeyboardState(NULL);
        input->isQuitPressed = 0;

        return input;
}

void InputDeinit(struct input *input) {
        if (NULL == input)
                return;

        free(input);
}

void InputProcess(struct input *input) {
        input->isQuitPressed = 0;
        SDL_PumpEvents(); // Update sdlKeyState;

        while (SDL_PollEvent(&input->event)) {
                switch (input->event.type) {
                        case SDL_QUIT:
                                input->isQuitPressed = 1;
                                break;

                        case SDL_KEYUP:
                                break;

                        case SDL_KEYDOWN:
                                if (input->event.key.keysym.sym == SDLK_ESCAPE) {
                                        input->isQuitPressed = 1;
                                        break;
                                }
                                break;
                }
        }
}

int InputIsQuitRequested(struct input *input) {
        return input->isQuitPressed;
}

int MapToSdlEnum(enum input_key_enum e) {
        switch(e) {
                case KEY_A:
                        return SDL_SCANCODE_A;
                case KEY_B:
                        return SDL_SCANCODE_B;
                case KEY_C:
                        return SDL_SCANCODE_C;
                case KEY_D:
                        return SDL_SCANCODE_D;
                case KEY_E:
                        return SDL_SCANCODE_E;
                case KEY_F:
                        return SDL_SCANCODE_F;
                case KEY_G:
                        return SDL_SCANCODE_G;
                case KEY_H:
                        return SDL_SCANCODE_H;
                case KEY_I:
                        return SDL_SCANCODE_I;
                case KEY_J:
                        return SDL_SCANCODE_J;
                case KEY_K:
                        return SDL_SCANCODE_K;
                case KEY_L:
                        return SDL_SCANCODE_L;
                case KEY_M:
                        return SDL_SCANCODE_M;
                case KEY_N:
                        return SDL_SCANCODE_N;
                case KEY_O:
                        return SDL_SCANCODE_O;
                case KEY_P:
                        return SDL_SCANCODE_P;
                case KEY_Q:
                        return SDL_SCANCODE_Q;
                case KEY_R:
                        return SDL_SCANCODE_R;
                case KEY_S:
                        return SDL_SCANCODE_S;
                case KEY_T:
                        return SDL_SCANCODE_T;
                case KEY_U:
                        return SDL_SCANCODE_U;
                case KEY_V:
                        return SDL_SCANCODE_V;
                case KEY_W:
                        return SDL_SCANCODE_W;
                case KEY_X:
                        return SDL_SCANCODE_X;
                case KEY_Y:
                        return SDL_SCANCODE_Y;
                case KEY_Z:
                        return SDL_SCANCODE_Z;
                case KEY_LEFT:
                        return SDL_SCANCODE_LEFT;
                case KEY_RIGHT:
                        return SDL_SCANCODE_RIGHT;
                case KEY_UP:
                        return SDL_SCANCODE_UP;
                case KEY_DOWN:
                        return SDL_SCANCODE_DOWN;
                case KEY_ENTER:
                        return SDL_SCANCODE_RETURN;
                case KEY_ESC:
                        return SDL_SCANCODE_ESCAPE;
                case KEY_SPACE:
                        return SDL_SCANCODE_SPACE;
                case KEY_1:
                        return SDL_SCANCODE_1;
                case KEY_2:
                        return SDL_SCANCODE_2;
                case KEY_3:
                        return SDL_SCANCODE_3;
                case KEY_4:
                        return SDL_SCANCODE_4;
                case KEY_5:
                        return SDL_SCANCODE_5;
                case KEY_6:
                        return SDL_SCANCODE_6;
                case KEY_7:
                        return SDL_SCANCODE_7;
                case KEY_8:
                        return SDL_SCANCODE_8;
                case KEY_9:
                        return SDL_SCANCODE_9;
                case KEY_0:
                        return SDL_SCANCODE_0;
                case KEY_LSHIFT:
                        return SDL_SCANCODE_LSHIFT;
                case KEY_RSHIFT:
                        return SDL_SCANCODE_RSHIFT;
                case KEY_LCTRL:
                        return SDL_SCANCODE_LCTRL;
                case KEY_RCTRL:
                        return SDL_SCANCODE_RCTRL;
                default:
                        return -1;
        };
        return -1;
}

unsigned int InputIsKeyPressed(struct input *input, enum input_key_enum e) {
        int i = MapToSdlEnum(e);
        if (i < 0) {
                return 0;
        }

        return input->sdlKeyStates[i];
}
