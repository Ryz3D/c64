#include "memory.h"

int8_t zeropage[256], stack[256], sysvar[512], screen[1024], cia2_reg[16];
int8_t basicram[38912];
// uint8_t ram[4096] = {0xd8, 0x58, 0xa0, 0x7f, 0xa5, 0xcb, 0xc9, 0x00, 0xf0, 0x17, 0xc9, 0x3e, 0xd0, 0x03, 0x4c, 0xf7, 0xc0, 0xc8, 0x10, 0x10, 0xa9, 0x2f, 0x20, 0xf4, 0xc0, 0xa9, 0x0d, 0xaa, 0x20, 0xf4, 0xc0, 0xa0, 0x01, 0x88, 0x30, 0xf5, 0xa5, 0xcb, 0xc5, 0x3b, 0xf0, 0xfa, 0x85, 0x3b, 0xc9, 0x40, 0xf0, 0xf4, 0xaa, 0xbd, 0x81, 0xeb, 0x99, 0x3c, 0x03, 0x20, 0xf4, 0xc0, 0x8a, 0xc9, 0x01, 0xd0, 0xc7, 0xa0, 0xff, 0xa9, 0x00, 0xaa, 0x0a, 0x0a, 0x85, 0xff, 0xc8, 0xb9, 0x3c, 0x03, 0xc9, 0x0d, 0xf0, 0xc9, 0xc9, 0x2e, 0x90, 0xf4, 0xf0, 0xee, 0xc9, 0x3a, 0xf0, 0xeb, 0xc9, 0x52, 0xf0, 0x3b, 0x86, 0xfc, 0x86, 0xfd, 0x84, 0xfe, 0xb9, 0x3c, 0x03, 0x49, 0x30, 0xc9, 0x0a, 0x90, 0x06, 0x69, 0x88, 0xc9, 0xfa, 0x90, 0x11, 0x0a, 0x0a, 0x0a, 0x0a, 0xa2, 0x04, 0x0a, 0x26, 0xfc, 0x26, 0xfd, 0xca, 0xd0, 0xf8, 0xc8, 0xd0, 0xe0, 0xc4, 0xfe, 0xf0, 0x8c, 0x24, 0xff, 0x50, 0x10, 0xa5, 0xfc, 0x81, 0xfa, 0xe6, 0xfa, 0xd0, 0xb5, 0xe6, 0xfb, 0x4c, 0x49, 0xc0, 0x6c, 0xf8, 0x00, 0x30, 0x2b, 0xa2, 0x02, 0xb5, 0xfb, 0x95, 0xf9, 0x95, 0xf7, 0xca, 0xd0, 0xf7, 0xd0, 0x14, 0xa9, 0x0d, 0x20, 0xf4, 0xc0, 0xa5, 0xf9, 0x20, 0xe1, 0xc0, 0xa5, 0xf8, 0x20, 0xe1, 0xc0, 0xa9, 0x3a, 0x20, 0xf4, 0xc0, 0xa9, 0x20, 0x20, 0xf4, 0xc0, 0xa1, 0xf8, 0x20, 0xe1, 0xc0, 0x86, 0xff, 0xa5, 0xf8, 0xc5, 0xfc, 0xa5, 0xf9, 0xe5, 0xfd, 0xb0, 0xc1, 0xe6, 0xf8, 0xd0, 0x02, 0xe6, 0xf9, 0xa5, 0xf8, 0x29, 0x07, 0x10, 0xc8, 0x48, 0x4a, 0x4a, 0x4a, 0x4a, 0x20, 0xea, 0xc0, 0x68, 0x29, 0x0f, 0x09, 0x30, 0xc9, 0x3a, 0x90, 0x02, 0x69, 0x06, 0x20, 0x16, 0xe7, 0x60, 0x00, 0x00};
bool charen;

int8_t mem_read(uint16_t addr)
{
    uint16_t addr_off = addr;
    if (addr < 0x0100)
    {
        // zeropage
        return zeropage[addr];
    }
    else if (addr < 0x0200)
    {
        addr_off -= 0x0100;
        // stack
        return stack[addr_off];
    }
    else if (addr < 0x0400)
    {
        addr_off -= 0x0200;
        // sysvar
        return sysvar[addr_off];
    }
    else if (addr < 0x0800)
    {
        addr_off -= 0x0400;
        // screen
        return screen[addr_off];
    }
    else if (addr < 0xa000)
    {
        addr_off -= 0x0800;
        // basic ram
        return basicram[addr_off];
    }
    else if (addr < 0xc000)
    {
        addr_off -= 0xa000;
        // basic rom
        return basic[addr_off];
    }
    else if (addr < 0xd000)
    {
        addr_off -= 0xc000;
        // free ram
        return ram[addr_off];
    }
    else if (addr < 0xe000)
    {
        if (charen)
        {
            if (addr < 0xd400)
            {
                addr_off -= 0xd000;
                // vic registers
                return vic_read(addr_off);
            }
            else if (addr < 0xd800)
            {
                addr_off -= 0xd400;
                // sid registers
            }
            else if (addr < 0xdc00)
            {
                addr_off -= 0xd800;
                // color ram
            }
            else if (addr < 0xdd00)
            {
                addr_off -= 0xdc00;
                return cia_read(addr_off % 16);
            }
            else if (addr < 0xde00)
            {
                addr_off -= 0xdd00;
                // cia 2
                return cia2_reg[addr_off % 16];
            }
            else
            {
                addr_off -= 0xde00;
                // interface expansions
            }
        }
        else
        {
            addr_off -= 0xd000;
            return characters[addr_off];
        }
    }
    else
    {
        addr_off -= 0xe000;
        // kernal rom
        return kernal[addr_off];
    }
    return 0;
}

uint16_t mem_read16(uint16_t addr0)
{
    return ((uint16_t)(uint8_t)mem_read(addr0 + 1) << 8) | (uint16_t)(uint8_t)mem_read(addr0);
}

void mem_write(uint16_t addr, int8_t d)
{
    uint16_t addr_off = addr;
    if (addr < 0x0100)
    {
        // zeropage
        zeropage[addr] = d;
        charen = zeropage[0x0001] & (1 << 2);
    }
    else if (addr < 0x0200)
    {
        addr_off -= 0x0100;
        // stack
        stack[addr_off] = d;
    }
    else if (addr < 0x0400)
    {
        addr_off -= 0x0200;
        // sysvar
        sysvar[addr_off] = d;
    }
    else if (addr < 0x0800)
    {
        addr_off -= 0x0400;
        // screen
        screen[addr_off] = d;
        vic_write_screen(addr_off, d);
    }
    else if (addr < 0xa000)
    {
        addr_off -= 0x0800;
        // basic ram
        basicram[addr_off] = d;
    }
    else if (addr < 0xc000)
    {
        addr_off -= 0xa000;
        // basic rom
    }
    else if (addr < 0xd000)
    {
        addr_off -= 0xc000;
        // free ram
        ram[addr_off] = d;
    }
    else if (addr < 0xd400)
    {
        addr_off -= 0xd000;
        // vic registers
        vic_write(addr_off, d);
    }
    else if (addr < 0xd800)
    {
        addr_off -= 0xd400;
        // sid registers
    }
    else if (addr < 0xdc00)
    {
        addr_off -= 0xd800;
        // color ram
    }
    else if (addr < 0xdd00)
    {
        addr_off -= 0xdc00;
        // cia 1
        cia_write(addr_off % 16, d);
    }
    else if (addr < 0xde00)
    {
        addr_off -= 0xdd00;
        // cia 2
        cia2_reg[addr_off % 16] = d;
    }
    else if (addr < 0xe000)
    {
        addr_off -= 0xde00;
        // interface expansions
    }
    else
    {
        addr_off -= 0xe000;
        // kernal rom
    }
}
