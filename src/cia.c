#include "cia.h"

/*
uint8_t columns[][8] = {
    {0x14, 0x0D, 0x1D, 0x88, 0x85, 0x86, 0x87, 0x11},
    {0x33, 0x57, 0x41, 0x34, 0x5A, 0x53, 0x45, 0x01},
    {0x35, 0x52, 0x44, 0x36, 0x43, 0x46, 0x54, 0x58},
    {0x37, 0x59, 0x47, 0x38, 0x42, 0x48, 0x55, 0x56},
    {0x39, 0x49, 0x4A, 0x30, 0x4D, 0x4B, 0x4F, 0x4E},
    {0x2B, 0x50, 0x4C, 0x2D, 0x2E, 0x3A, 0x40, 0x2C},
    {0x5C, 0x2A, 0x3B, 0x13, 0x01, 0x3D, 0x5E, 0x2F},
    {0x31, 0x5F, 0x04, 0x32, 0x20, 0x02, 0x51, 0x03}};
*/

// 219 = ß
uint8_t columns[][8] = {
    {VK_BACK, VK_RETURN, VK_RIGHT, VK_F7, VK_F1, VK_F3, VK_F5, VK_DOWN},
    {0x33, 0x57, 0x41, 0x34, 0x5A, 0x53, 0x45, VK_LSHIFT},
    {0x35, 0x52, 0x44, 0x36, 0x43, 0x46, 0x54, 0x58},
    {0x37, 0x59, 0x47, 0x38, 0x42, 0x48, 0x55, 0x56},
    {0x39, 0x49, 0x4A, 0x30, 0x4D, 0x4B, 0x4F, 0x4E},
    {VK_OEM_PLUS, 0x50, 0x4C, VK_OEM_MINUS, VK_OEM_PERIOD, VK_ADD, 0x40, VK_OEM_COMMA},
    {0x5C, VK_MULTIPLY, VK_SUBTRACT, 0x13, VK_RSHIFT, 219, VK_UP, VK_DIVIDE},
    {0x31, VK_LEFT, VK_CONTROL, 0x32, VK_SPACE, VK_TAB, 0x51, VK_ESCAPE}};

/*
// windows constants
uint8_t columns[][8] = {
    {0x08, 0x0D, 0x27, 0x76, 0x70, 0x72, 0x74, 0x28},
    {0x33, 0x57, 0x41, 0x34, 0x5A, 0x53, 0x45, 0x01},
    {0x35, 0x52, 0x44, 0x36, 0x43, 0x46, 0x54, 0x58},
    {0x37, 0x59, 0x47, 0x38, 0x42, 0x48, 0x55, 0x56},
    {0x39, 0x49, 0x4A, 0x30, 0x4D, 0x4B, 0x4F, 0x4E},
    {0xBB, 0x50, 0x4C, 0xBD, 0xBE, 0x6B, 0x40, 0xBC},
    {0x5C, 0x6A, 0x6D, 0x13, 0xA1, 0xDB, 0x26, 0x6F},
    {0x31, 0x25, 0x11, 0x32, 0x20, 0x09, 0x51, 0x1B}};
*/

uint8_t port_a = 0;

int8_t cia_read(uint8_t addr_off)
{
    if (addr_off == 0x00)
        return port_a;
    else if (addr_off == 0x01)
    {
        uint8_t matrix = 0xFF;
        for (uint8_t c = 0; c < 8; c++)
            if ((~port_a >> c) & 1)
                for (uint8_t r = 0; r < 8; r++)
                    for (uint8_t i = 0; i < 10; i++)
                        if (GetKeyState(columns[c][r]) >> 15)
                            matrix &= ~(1 << r);
        return matrix;
    }
    return 0;
}

void cia_write(uint8_t addr_off, int8_t d)
{
    if (addr_off == 0x00)
        port_a = d;
}
