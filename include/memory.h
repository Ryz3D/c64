#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "basic.h"
#include "characters.h"
#include "kernal.h"
#include "cia.h"
#include "vic.h"
#include "ram.h"

extern int8_t zeropage[256], stack[256], sysvar[512], screen[1024], cia2_reg[16];
extern int8_t basicram[38912];
extern bool charen;

int8_t mem_read(uint16_t addr);
uint16_t mem_read16(uint16_t addr);
void mem_write(uint16_t addr, int8_t d);
