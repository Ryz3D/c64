#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "characters.h"
#include "simple_draw.h"

extern uint8_t vic_reg[48];
extern uint8_t vic_line;

int8_t vic_read(uint16_t addr_off);
void vic_write(uint16_t addr_off, int8_t d);
void vic_write_screen(uint16_t addr_off, int8_t d);
