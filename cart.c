/******************************************************************************
  GrooveStomp's NES Emulator
  Copyright (c) 2019 Aaron Oman (GrooveStomp)

  File: cart.c
  Created: 2019-11-03
  Updated: 2019-11-04
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
        uint8_t mapper_id;
        uint8_t prg_banks;
        uint8_t chr_banks;
        bool is_image_valid;
        uint8_t *prg_mem;
        uint8_t *chr_mem;
        void *mapper;
        map_cpu_read_fn map_cpu_read;
        map_cpu_write_fn map_cpu_write;
        map_ppu_read_fn map_ppu_read;
        map_ppu_write_fn map_ppu_write;
};

struct header {
        char name[4];
        uint8_t prg_rom_chunks;
        uint8_t chr_rom_chunks;
        uint8_t mapper1;
        uint8_t mapper2;
        uint8_t prg_ram_size;
        uint8_t tv_system1;
        uint8_t tv_system2;
        char unused[5];
};

struct cart *CartInit(char *filename) {
        struct cart *cart = (struct cart *)malloc(sizeof(struct cart));
        if (NULL == cart) {
                return NULL;
        }

        cart->is_image_valid = false;
        cart->mapper_id = 0;
        cart->prg_banks = 0;
        cart->chr_banks = 0;

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
        cart->mapper_id = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);

        // "Discover" file format.
        uint8_t file_type = 1;
        if (0 == file_type) {
                // TODO unhandled file_type
        }

        if (1 == file_type) {
                cart->prg_banks = header.prg_rom_chunks;
                cart->prg_mem = (uint8_t *)malloc(sizeof(uint8_t) * KB_AS_B(16));
                objs_read = fread(cart->prg_mem, 1, KB_AS_B(16), f);
                if (objs_read < KB_AS_B(16)) {
                        fclose(f);
                        free(cart);
                        return NULL;
                        // TODO Couldn't read data from file - set appropriate error
                }

                cart->chr_banks = header.chr_rom_chunks;
                cart->chr_mem = (uint8_t *)malloc(sizeof(uint8_t) * KB_AS_B(8));
                objs_read = fread(cart->chr_mem, 1, KB_AS_B(8), f);
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

        switch(cart->mapper_id) {
                case 0: {
                        cart->mapper = Mapper000_Init(cart->prg_banks, cart->chr_banks);
                        if (NULL == cart->mapper) {
                                // TODO handle cart->mapper not being initialized
                        }
                        cart->map_cpu_read = Mapper000_MapCpuRead;
                        cart->map_cpu_write = Mapper000_MapCpuWrite;
                        cart->map_ppu_read = Mapper000_MapPpuRead;
                        cart->map_ppu_write = Mapper000_MapPpuWrite;
                        break;
                }
        }

        cart->is_image_valid = true;
        fclose(f);

        return cart;
}

void CartDeinit(struct cart *cart) {
        if (NULL == cart)
                return;

        if (NULL != cart->chr_mem)
                free(cart->chr_mem);

        if (NULL != cart->prg_mem)
                free(cart->prg_mem);

        free(cart);
}

bool CartCpuRead(struct cart *cart, uint16_t addr, uint8_t *data) {
        uint32_t mapped_addr = 0;
        if (cart->map_cpu_read(cart->mapper, addr, &mapped_addr)) {
                *data = cart->prg_mem[mapped_addr];
                return true;
        }

        return false;
}

bool CartCpuWrite(struct cart *cart, uint16_t addr, uint8_t data) {
        uint32_t mapped_addr = 0;
        if (cart->map_cpu_write(cart->mapper, addr, &mapped_addr)) {
                cart->prg_mem[mapped_addr] = data;
                return true;
        }

        return false;
}

bool CartPpuRead(struct cart *cart, uint16_t addr, uint8_t *data) {
        uint32_t mapped_addr = 0;
        if (cart->map_ppu_read(cart->mapper, addr, &mapped_addr)) {
                *data = cart->chr_mem[mapped_addr];
                return true;
        }

        return false;
}

bool CartPpuWrite(struct cart *cart, uint16_t addr, uint8_t data) {
        uint32_t mapped_addr = 0;
        if (cart->map_ppu_write(cart->mapper, addr, &mapped_addr)) {
                cart->chr_mem[mapped_addr] = data;
                return true;
        }

        return false;
}
