/******************************************************************************
  GrooveStomp's Text Renderer
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
        memset(input, 0, sizeof(struct input));

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
