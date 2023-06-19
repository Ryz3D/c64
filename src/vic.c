#include "vic.h"

uint8_t vic_reg[48];
uint8_t vic_line = 0;

int8_t vic_read(uint16_t addr_off)
{
    if (addr_off == 0x12)
        return vic_line = (vic_line + 1) % 24;
    else if (addr_off == 0x19)
        return 0; // NTSC
    else
        return vic_reg[addr_off % 0x30];
}

void vic_write(uint16_t addr_off, int8_t d)
{
    vic_reg[addr_off % 0x30] = d;
}

void vic_write_screen(uint16_t addr_off, int8_t d)
{
    uint8_t screenY = addr_off / 40;
    if (screenY <= 24)
    {
        uint8_t screenX = addr_off % 40;
        for (uint8_t charY = 0; charY < 8; charY++)
        {
            uint8_t charLine = characters[(uint8_t)d * 8 + charY + ((uint8_t)vic_reg[0x18] >> 1 ? 0x0000 : 0x0800)];
            for (uint8_t charX = 0; charX < 8; charX++)
            {
                uint16_t windowX = 8 * (uint16_t)screenX + charX; // 0...319
                uint16_t windowY = 8 * (uint16_t)screenY + charY; // 0...199
                if ((charLine >> (7 - charX)) & 1)
                    DrawPixel(windowX, windowY, 112, 108, 230);
                else
                    DrawPixel(windowX, windowY, 39, 33, 207);
            }
        }
    }
}
