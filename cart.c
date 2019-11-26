/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: cart.c
  Created: 2019-11-03
  Updated: 2019-11-21
  Author: Aaron Oman
  Notice: GNU AGPLv3 License

  Based off of: One Lone Coder NES Emulator Copyright (C) 2019 Javidx9
  This program comes with ABSOLUTELY NO WARRANTY.
  This is free software, and you are welcome to redistribute it under certain
  conditions; See LICENSE for details.
 ******************************************************************************/
//! \file cart.c
#include <stdint.h>
#include <stdio.h> // fopen
#include <stdbool.h> // bool
#include <stdlib.h> // malloc

#include "cart.h"
#include "util.h"
#include "mapper.h"
#include "mapper000.h"

struct cart {
        uint8_t mapperId;
        uint8_t prgBanks;
        uint8_t chrBanks;
        bool isImageValid;
        uint8_t *prgMem;
        uint8_t *chrMem;
        void *mapper;
        map_cpu_read_fn mapCpuRead;
        map_cpu_write_fn mapCpuWrite;
        map_ppu_read_fn mapPpuRead;
        map_ppu_write_fn mapPpuWrite;

        enum mirror mirror;
};

struct header {
        char name[4];
        uint8_t prgRomChunks;
        uint8_t chrRomChunks;
        uint8_t mapper1;
        uint8_t mapper2;
        uint8_t prgRamSize;
        uint8_t tvSystem1;
        uint8_t tvSystem2;
        char unused[5];
};

struct cart *CartInit(char *filename) {
        struct cart *cart = (struct cart *)malloc(sizeof(struct cart));
        if (NULL == cart) {
                return NULL;
        }

        cart->isImageValid = false;
        cart->mapperId = 0;
        cart->prgBanks = 0;
        cart->chrBanks = 0;
        cart->mirror = MIRROR_HORIZONTAL;

        FILE *f = fopen(filename, "rb");
        if (NULL == f) {
                free(cart);
                return NULL;
        }
        struct header header;
        size_t objs_read = fread(&header, sizeof(struct header), 1, f);
        if (objs_read != 1) {
                fclose(f);
                free(cart);
                return NULL;
        }

        // For this mapper, the next 512 bytes are trainer info; which we ignore.
        if (header.mapper1 & 0x04) {
                fseek(f, 512, SEEK_CUR);
        }

        // Determine mapper ID
        cart->mapperId = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
        cart->mirror = (header.mapper1 & 0x01) ? MIRROR_VERTICAL : MIRROR_HORIZONTAL;

        // "Discover" file format.
        uint8_t file_type = 1;
        if (0 == file_type) {
                // TODO unhandled file_type
        }

        if (1 == file_type) {
                cart->prgBanks = header.prgRomChunks;
                cart->prgMem = (uint8_t *)malloc(sizeof(uint8_t) * KB_AS_B(16));
                objs_read = fread(cart->prgMem, 1, KB_AS_B(16), f);
                if (objs_read < KB_AS_B(16)) {
                        fclose(f);
                        free(cart);
                        return NULL;
                        // TODO Couldn't read data from file - set appropriate error
                }

                cart->chrBanks = header.chrRomChunks;
                cart->chrMem = (uint8_t *)malloc(sizeof(uint8_t) * KB_AS_B(8));
                objs_read = fread(cart->chrMem, 1, KB_AS_B(8), f);
                if (objs_read < KB_AS_B(8)) {
                        fclose(f);
                        free(cart);
                        return NULL;
                        // TODO Couldn't read data from file - set appropriate error
                }
        }

        if (2 == file_type) {
                // TODO unhandled file_type
        }

        switch(cart->mapperId) {
                case 0: {
                        cart->mapper = Mapper000_Init(cart->prgBanks, cart->chrBanks);
                        if (NULL == cart->mapper) {
                                // TODO handle cart->mapper not being initialized
                        }
                        cart->mapCpuRead = Mapper000_MapCpuRead;
                        cart->mapCpuWrite = Mapper000_MapCpuWrite;
                        cart->mapPpuRead = Mapper000_MapPpuRead;
                        cart->mapPpuWrite = Mapper000_MapPpuWrite;
                        break;
                }
        }

        cart->isImageValid = true;
        fclose(f);

        return cart;
}

void CartDeinit(struct cart *cart) {
        if (NULL == cart)
                return;

        if (NULL != cart->chrMem)
                free(cart->chrMem);

        if (NULL != cart->prgMem)
                free(cart->prgMem);

        free(cart);
}

bool CartCpuRead(struct cart *cart, uint16_t addr, uint8_t *data) {
        uint32_t mapped_addr = 0;
        if (cart->mapCpuRead(cart->mapper, addr, &mapped_addr)) {
                *data = cart->prgMem[mapped_addr];
                return true;
        }

        return false;
}

bool CartCpuWrite(struct cart *cart, uint16_t addr, uint8_t data) {
        uint32_t mapped_addr = 0;
        if (cart->mapCpuWrite(cart->mapper, addr, &mapped_addr)) {
                cart->prgMem[mapped_addr] = data;
                return true;
        }

        return false;
}

bool CartPpuRead(struct cart *cart, uint16_t addr, uint8_t *data) {
        uint32_t mapped_addr = 0;
        if (cart->mapPpuRead(cart->mapper, addr, &mapped_addr)) {
                *data = cart->chrMem[mapped_addr];
                return true;
        }

        return false;
}

bool CartPpuWrite(struct cart *cart, uint16_t addr, uint8_t data) {
        uint32_t mapped_addr = 0;
        if (cart->mapPpuWrite(cart->mapper, addr, &mapped_addr)) {
                cart->chrMem[mapped_addr] = data;
                return true;
        }

        return false;
}

enum mirror CartMirroring(struct cart *cart) {
        return cart->mirror;
}
