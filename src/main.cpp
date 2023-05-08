#include "main.h"

/*

TODO:
- cout on jmp to $ffd2
- code injection into free ram

*/

// $0001
bool charen, hiram, loram;
uint8_t stack_pointer;
int8_t zeropage[256], stack[256], sysvar[512];
int8_t basicram[38912], ram[4096] = {
                            0x00,
};
uint16_t pc;
int8_t a, x, y;
bool fN, fV, fB, fD, fI, fZ, fC;

uint8_t s_pop()
{
    return stack[stack_pointer++];
}

void s_push(int8_t d)
{
    stack[stack_pointer--] = d;
}

int8_t read(uint16_t addr)
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
        // SCREEN
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
                // vic (allowed up to d02e) registers
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

void write(uint16_t addr, int8_t d)
{
    uint16_t addr_off = addr;
    if (addr < 0x0100)
    {
        // zeropage
        zeropage[addr] = d;
        charen = zeropage[0x0001] & (1 << 2);
        hiram = zeropage[0x0001] & (1 << 1);
        loram = zeropage[0x0001] & (1 << 0);
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
        // SCREEN
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
    return ((uint16_t)read(addr0 + 1) << 8) | (uint16_t)read(addr0);
}

int8_t *ins_buf(uint8_t len)
{
    int8_t *buf = (int8_t *)malloc(len);
    for (uint16_t i = 0; i < len; i++)
        buf[i] = read(pc + i);
    pc += len;
    return buf;
}

bool get_operand(int8_t *op, uint16_t *op_addr, uint8_t ins, uint8_t *variants)
{
    if (ins == 0x00)
        return 0;

    int8_t *buf = nullptr;
    if (ins == variants[0])
    {
        // immediate
        *op = ins_buf(2)[1];
    }
    else if (ins == variants[1])
    {
        // zeropage
        *op_addr = (uint8_t)ins_buf(2)[1];
        *op = read(*op_addr);
    }
    else if (ins == variants[2])
    {
        // zeropage x
        *op_addr = (uint8_t)ins_buf(2)[1] + x;
        *op = read(*op_addr);
    }
    else if (ins == variants[3])
    {
        // zeropage y
        *op_addr = (uint8_t)ins_buf(2)[1] + y;
        *op = read(*op_addr);
    }
    else if (ins == variants[4])
    {
        // indirect zeropage x
        uint8_t op_addr_addr = (uint8_t)ins_buf(2)[1] + x;
        *op_addr = read16(op_addr_addr);
        *op = read(*op_addr);
    }
    else if (ins == variants[5])
    {
        // indirect zeropage y
        uint8_t op_addr_addr = (uint8_t)ins_buf(2)[1];
        *op_addr = read16(op_addr_addr) + y;
        *op = read(*op_addr);
    }
    else if (ins == variants[6])
    {
        // absolute
        int8_t *buf = ins_buf(3);
        *op_addr = ((uint16_t)buf[2] << 8) | (uint16_t)buf[1];
        *op = read(*op_addr);
    }
    else if (ins == variants[7])
    {
        // absolute indexed x
        int8_t *buf = ins_buf(3);
        *op_addr = (((uint16_t)buf[2] << 8) | (uint16_t)buf[1]) + x;
        *op = read(*op_addr);
    }
    else if (ins == variants[8])
    {
        // absolute indexed y
        int8_t *buf = ins_buf(3);
        *op_addr = (((uint16_t)buf[2] << 8) | (uint16_t)buf[1]) + y;
        *op = read(*op_addr);
    }
  else if (ins == variants[9])
  {
    // indirect
        int8_t *buf = ins_buf(3);
        uint16_t op_addr_addr = ((uint16_t)buf[2] << 8) | (uint16_t)buf[1];
        *op_addr = read16(op_addr_addr);
        *op = read(*op_addr);
  }
  else if (ins == variants[10])
  {
    // relative
      uint16_t base_pc=pc;
        *op_addr = base_pc+ins_buf(2)[1];
        *op = read(*op_addr);
  }
    else
        return 0;

    if (buf != nullptr)
        free(buf);
    return 1;
}

uint16_t useless_addr;

bool exec_ora(uint8_t ins, int8_t *res)
{
    int8_t op;
    if (get_operand(&op, &useless_addr, ins, new uint8_t[11]{0x09, 0x05, 0x15, 0x00, 0x01, 0x11, 0x0d, 0x1d, 0x19,0x00,0x00}))
    {
        *res = a |= op;
        return 1;
    }
    return 0;
}

bool exec_and(uint8_t ins, int8_t *res)
{
    int8_t op;
    if (get_operand(&op, &useless_addr, ins, new uint8_t[11]{0x29, 0x25, 0x35, 0x00, 0x21, 0x31, 0x2D, 0x3D, 0x39,0x00,0x00}))
    {
        *res = a &= op;
        return 1;
    }
    return 0;
}

bool exec_eor(uint8_t ins, int8_t *res)
{
    int8_t op;
    if (get_operand(&op, &useless_addr, ins, new uint8_t[11]{0x49, 0x45, 0x55, 0x00, 0x41, 0x51, 0x4D, 0x5D, 0x59,0x00,0x00}))
    {
        *res = a ^= op;
        return 1;
    }
    return 0;
}

bool exec_adc(uint8_t ins, int8_t *res)
{
    int8_t op;
    if (get_operand(&op, &useless_addr, ins, new uint8_t[11]{0x69, 0x65, 0x75, 0x00, 0x61, 0x71, 0x6D, 0x7D, 0x79,0x00,0x00}))
    {
        *res = a += op + fC;
        // TODO: fV, fC
        return 1;
    }
    return 0;
}

bool exec_sbc(uint8_t ins, int8_t *res)
{
    int8_t op;
    if (get_operand(&op, &useless_addr, ins, new uint8_t[11]{0xe9, 0xe5, 0xf5, 0x00, 0xe1, 0xf1, 0xeD, 0xfD, 0xf9,0x00,0x00}))
    {
        // TODO: fC in
        *res = a -= op - fC;
        // TODO: fV, fC
        return 1;
    }
    return 0;
}

bool exec_cmp(uint8_t ins, int8_t *res)
{
    int8_t op;
    if (get_operand(&op, &useless_addr, ins, new uint8_t[11]{0xc9,0xc5,0xd5,0x00,0xc1,0xd1,0xcd,0xdd,0xd9,0x00,0x00}))
    {
        // TODO: fC in
        *res = a - op - fC;
        // TODO: fV, fC
        return 1;
    }
    return 0;
}

bool exec_cpx(uint8_t ins, int8_t *res)
{
    int8_t op;
    if (get_operand(&op, &useless_addr, ins, new uint8_t[11]{0xe0,0xe4,0x00,0x00,0x00,0x00,0xec,0x00,0x00,0x00,0x00}))
    {
        // TODO: fC in
        *res = x - op - fC;
        // TODO: fV, fC
        return 1;
    }
    return 0;
}

bool exec_cpy(uint8_t ins, int8_t *res)
{
    int8_t op;
    if (get_operand(&op, &useless_addr, ins, new uint8_t[11]{0xc0,0xc4,0x00,0x00,0x00,0x00,0xcc,0x00,0x00,0x00,0x00}))
    {
        // TODO: fC in
        *res = y - op - fC;
        // TODO: fV, fC
        return 1;
    }
    return 0;
}

bool exec_dec(uint8_t ins, int8_t *res)
{
    int8_t op;
    uint16_t op_addr;
    if (get_operand(&op, &op_addr, ins, new uint8_t[11]{0x00,0xc6,0xd6,0x00,0x00,0x00,0xce,0xde,0x00,0x00,0x00}))
    {
        write(op_addr, *res=op-1);
        return 1;
    }
    return 0;
}

bool exec_inc(uint8_t ins, int8_t *res)
{
    int8_t op;
    uint16_t op_addr;
    if (get_operand(&op, &op_addr, ins, new uint8_t[11]{0x00,0xe6,0xf6,0x00,0x00,0x00,0xee,0xfe,0x00,0x00,0x00}))
    {
        write(op_addr, *res=op+1);
        return 1;
    }
    return 0;
}

bool exec_asl(uint8_t ins, int8_t *res)
{
    int8_t op;
    uint16_t op_addr;
    if (get_operand(&op, &op_addr, ins, new uint8_t[11]{0x00,0x06,0x16,0x00,0x00,0x00,0x0e,0x1e,0x00,0x00,0x00}))
    {
      fC=op>>7;
        write(op_addr, *res=op<<1);
        return 1;
    }
    return 0;
}

bool exec_rol(uint8_t ins, int8_t *res)
{
    int8_t op;
    uint16_t op_addr;
    if (get_operand(&op, &op_addr, ins, new uint8_t[11]{0x00,0x26,0x36,0x00,0x00,0x00,0x2e,0x3e,0x00,0x00,0x00}))
    {
    bool c_in=fC;
      fC=op>>7;
        write(op_addr, *res=(op<<1)|c_in);
        return 1;
    }
    return 0;
}

bool exec_lsr(uint8_t ins, int8_t *res)
{
    int8_t op;
    uint16_t op_addr;
    if (get_operand(&op, &op_addr, ins, new uint8_t[11]{0x00,0x46,0x56,0x00,0x00,0x00,0x4e,0x5e,0x00,0x00,0x00}))
    {
      fC=op&1;
        write(op_addr, *res=op>>1);
        return 1;
    }
    return 0;
}

bool exec_ror(uint8_t ins, int8_t *res)
{
    int8_t op;
    uint16_t op_addr;
    if (get_operand(&op, &op_addr, ins, new uint8_t[11]{0x00,0x66,0x76,0x00,0x00,0x00,0x6e,0x7e,0x00,0x00,0x00}))
    {
    bool c_in=fC;
      fC=op&1;
        write(op_addr, *res=(op>>1)|(c_in<<7));
        return 1;
    }
    return 0;
}

void exec_ins()
{
    uint8_t ins = (uint8_t)read(pc);
    int8_t res = 1;
    bool skipZN = 0;

    if (exec_ora(ins, &res))
        ;
    else if (exec_and(ins, &res))
        ;
    else if (exec_eor(ins, &res))
        ;
    else if (exec_adc(ins, &res))
        ;
    else if (exec_sbc(ins, &res))
        ;
    else if (exec_cmp(ins, &res))
        ;
    else if (exec_cpx(ins, &res))
        ;
    else if (exec_cpy(ins, &res))
        ;
    else if (exec_dec(ins, &res))
        ;
    else if (ins == 0xca)
      res = x--;
    else if (ins == 0x88)
      res = y--;
    else if (exec_inc(ins, &res))
        ;
    else if (ins == 0xe8)
      res = x++;
    else if (ins == 0xc8)
      res = y++;
    else if (ins == 0x0a)
      {
      fC=a>>7;
      res=a = a << 1;
      }
    else if (exec_asl(ins, &res))
        ;
    else if (ins == 0x2a)
      {
    bool c_in=fC;
      fC=a>>7;
      res = a = (a<<1)|c_in;
      }
    else if (exec_rol(ins, &res))
        ;
    else if (ins == 0x4a)
      {
      fC=a&1;
      res = a = a >> 1;
      }
    else if (exec_lsr(ins, &res))
        ;
    else if (ins == 0x6a)
      {
    bool c_in=fC;
      fC=a&1;
      res = a = (a >> 1) | (c_in << 7);
      }
    else if (exec_ror(ins, &res))
        ;
    else if (ins == 0xaa)
        res = x = a;
    else if (ins == 0x8a)
        res = a = x;
    else if (ins == 0xa8)
        res = y = a;
    else if (ins == 0x98)
        res = a = y;
    else if (ins == 0xba)
        res = x = stack_pointer;
    else if (ins == 0x9a)
    {
        stack_pointer = x;
        skipZN = 1;
    }
    else if (ins == 0x68)
        res = a = s_pop();
    else if (ins == 0x48)
    {
        s_push(a);
        skipZN = 1;
    }
    else if (ins == 0x28)
    {
        uint8_t status = res = s_pop();
        fN = status & (1 << 7);
        fV = status & (1 << 6);
        // fB = status & (1 << 4);
        fD = status & (1 << 3);
        fI = status & (1 << 2);
        fZ = status & (1 << 1);
        fC = status & (1 << 0);
    }
    else if (ins == 0x08)
    {
        s_push((fN << 7) | (fV << 6) | (1 << 5) | (fB << 4) | (fD << 3) | (fI << 2) | (fZ << 1) | (fC << 0));
        skipZN = 1;
    }
    else
        std::cout << "unknown instruction " << ins << std::endl;

    if (!skipZN)
    {
        fZ = res == 0;
        fN = res & (1 << 7);
    }
}

void reset()
{
    stack_pointer = 0xfd;
    a = 0, x = 0, y = 0;
    fN = 0, fV = 0, fB = 0, fD = 0, fI = 0, fZ = 0, fC = 0;
    pc = read16(0xfffc);
}

void nmi()
{
    /*
    TODO: The first thing the CPU does in response to both types of interrupts is to save the program counter and the status register onto the stack for retrieval after servicing the interrupt. For both types of interrupt, along with the cold start, the CPU is hardwired to perform what amounts to an indirect JMP via one of three vectors stored in the very last six bytes of KERNAL ROM:
    */
    pc = read16(0xfffa);
}

void irq()
{
    pc = read16(0xfffe);
}

int main(int argc, char *argv[])
{
    reset();
    pc = 0xc000;

    std::cout << std::hex << a;
    std::cout << pc << std::endl;

    exec_ins();

    std::cout << std::hex << a;
    std::cout << pc << std::endl;

    exec_ins();

    std::cout << std::hex << a;
    std::cout << pc << std::endl;

    return 0;
}
