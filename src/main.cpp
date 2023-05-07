#include "main.h"

/*

vic d000-d02e nur 47? theoretisch -d3ff
sid d400-d7ff
col d800-dbff
cia1 dc00-dcff
cia2 dd00-ddff
io1 de00-deff
io2 df00-dfff

*/

// cout on jmp to $ffd2

// $0001
bool charen = 1, hiram = 0, loram = 0;
uint8_t stack_pointer = 255;
uint8_t stack[256], sysvar[512];
uint8_t basicram[38912];
uint8_t ram[4096];
uint16_t pc = 0;
uint8_t a = 0, x = 0, y = 0;

uint8_t s_pop()
{
    return stack[stack_pointer++];
}

void s_push(uint8_t d)
{
    stack[stack_pointer--] = d;
}

uint8_t read(uint16_t addr)
{
    uint16_t addr_off = addr;
    if (addr < 0x0100)
    {
        // zeropage
        if (addr == 0x0000)
            return 0b00101111;
        if (addr == 0x0001)
            return 0b11110000 | (charen << 2) | (hiram << 1) | (loram << 0);
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
    }
    else if (addr < 0x0800)
    {
        addr_off -= 0x0400;
        // SCREEN
    }
    else if (addr < 0xa000)
    {
        addr_off -= 0x0800;
        // basic ram
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
    }
    else if (addr < 0xd400)
    {
        addr_off -= 0xd000;
        // vic (allowed to d02e) registers
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
    else if (addr < 0xde00)
    {
        addr_off -= 0xdc00;
        // cia registers
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
        return kernal[addr_off];
    }
    return 0;
}

void write(uint16_t addr, uint8_t d)
{
    uint16_t addr_off = addr;
    if (addr < 0x0100)
    {
        // zeropage
        if (addr == 0x0001)
        {
            charen = d & (1 << 2);
            hiram = d & (1 << 1);
            loram = d & (1 << 0);
        }
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
    }
    else if (addr < 0x0800)
    {
        addr_off -= 0x0400;
        // SCREEN
    }
    else if (addr < 0xa000)
    {
        addr_off -= 0x0800;
        // basic ram
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
    }
    else if (addr < 0xd400)
    {
        addr_off -= 0xd000;
        // vic (allowed to d02e) registers
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
    else if (addr < 0xde00)
    {
        addr_off -= 0xdc00;
        // cia registers
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

uint16_t read16(uint16_t addr0)
{
    return (read(addr0 + 1) << 8) | read(addr0);
}

uint8_t *ins_buf(uint8_t len)
{
    uint8_t *buf = (uint8_t *)malloc(len);
    for (uint16_t i = 0; i < len; i++)
        buf[i] = read(pc + i);
    pc += len;
    return buf;
}

void exec_and(uint8_t ins)
{
    if (ins == 0x09)
    {
        uint8_t *buf = ins_buf(2);
    }
    else if (ins == 0xa0)
    {
        uint8_t *buf = ins_buf(2);
    }
}

void exec_ins()
{
    uint8_t *ins_buf;
    if (ins == 0x09 || ins == 0xa0)
        pc += exec_and(ins);
    else
        std::cout << "unknown ins " << ins_buf[0] << std::endl;
}

void reset()
{
    /*
    todo: The first thing the CPU does in response to both types of interrupts is to save the program counter and the status register onto the stack for retrieval after servicing the interrupt. For both types of interrupt, along with the cold start, the CPU is hardwired to perform what amounts to an indirect JMP via one of three vectors stored in the very last six bytes of KERNAL ROM:
    */
    pc = read16(0xfffc);
}

void nmi()
{
    pc = read16(0xfffa);
}

void irq()
{
    pc = read16(0xfffe);
}

int main(int argc, char *argv[])
{
    reset();

    std::cout << std::hex << a;
    std::cout << pc << std::endl;

    return 0;
}
