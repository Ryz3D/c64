#include "cpu.h"

uint16_t pc;
int8_t a, x, y;
bool fN, fV, fB, fD, fI, fZ, fC;
uint8_t stack_pointer;
int8_t ins_buf[3];

void reset()
{
    stack_pointer = 0xff;
    fN = 0, fV = 0, fB = 0, fD = 0, fI = 0, fZ = 0, fC = 0;
    pc = mem_read16(0xfffc);
}

void irq()
{
    if (!fI)
    {
        s_push16(pc);
        fB = 0;
        s_push((fN << 7) | (fV << 6) | (1 << 5) | (fB << 4) | (fD << 3) | (fI << 2) | (fZ << 1) | (fC << 0));
        fI = 1;
        pc = mem_read16(0xfffe);
    }
}

uint8_t s_pop()
{
    return stack[++stack_pointer];
}

void s_push(int8_t d)
{
    stack[stack_pointer--] = d;
}

uint16_t s_pop16()
{
    uint8_t a = s_pop();
    uint8_t b = s_pop();
    return ((uint16_t)b << 8) | (uint16_t)a;
}

void s_push16(uint16_t d)
{
    s_push(d >> 8);
    s_push(d & 0xff);
}

void load_ins_buf(uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
        ins_buf[i] = mem_read(pc + i);
    pc += len;
}

bool get_operand(int8_t *op, uint16_t *op_addr, uint8_t ins, uint8_t *variants)
{
    if (ins == 0x00)
        return 0;

    if (ins == variants[0])
    {
        // immediate
        load_ins_buf(2);
        *op = ins_buf[1];
    }
    else
    {
        if (ins == variants[1])
        {
            // zeropage
            load_ins_buf(2);
            *op_addr = (uint8_t)ins_buf[1];
        }
        else if (ins == variants[2])
        {
            // x-indexed zeropage
            load_ins_buf(2);
            *op_addr = (uint8_t)((uint8_t)ins_buf[1] + (uint8_t)x);
        }
        else if (ins == variants[3])
        {
            // y-indexed zeropage
            load_ins_buf(2);
            *op_addr = (uint8_t)((uint8_t)ins_buf[1] + (uint8_t)y);
        }
        else if (ins == variants[4])
        {
            // x-indexed zeropage indirect
            load_ins_buf(2);
            uint8_t op_addr_addr = (uint8_t)ins_buf[1] + (uint8_t)x;
            *op_addr = mem_read16(op_addr_addr);
        }
        else if (ins == variants[5])
        {
            // y-indexed zeropage indirect
            load_ins_buf(2);
            uint8_t op_addr_addr = (uint8_t)ins_buf[1];
            *op_addr = mem_read16(op_addr_addr) + (uint8_t)y;
        }
        else if (ins == variants[6])
        {
            // absolute
            load_ins_buf(3);
            *op_addr = ((uint16_t)(uint8_t)ins_buf[2] << 8) | (uint16_t)(uint8_t)ins_buf[1];
        }
        else if (ins == variants[7])
        {
            // x-indexed absolute
            load_ins_buf(3);
            *op_addr = (((uint16_t)(uint8_t)ins_buf[2] << 8) | (uint16_t)(uint8_t)ins_buf[1]) + (uint8_t)x;
        }
        else if (ins == variants[8])
        {
            // y-indexed absolute
            load_ins_buf(3);
            *op_addr = (((uint16_t)(uint8_t)ins_buf[2] << 8) | (uint16_t)(uint8_t)ins_buf[1]) + (uint8_t)y;
        }
        else if (ins == variants[9])
        {
            // absolute indirect
            load_ins_buf(3);
            uint16_t op_addr_addr = ((uint16_t)(uint8_t)ins_buf[2] << 8) | (uint16_t)(uint8_t)ins_buf[1];
            *op_addr = mem_read16(op_addr_addr);
        }
        else if (ins == variants[10])
        {
            // relative
            load_ins_buf(2);
            *op_addr = pc + ins_buf[1];
        }
        else
            return 0;

        *op = mem_read(*op_addr);
    }

    return 1;
}

uint16_t useless_addr;

uint8_t v_ora[] = {0x09, 0x05, 0x15, 0x00, 0x01, 0x11, 0x0d, 0x1d, 0x19, 0x00, 0x00};
uint8_t v_and[] = {0x29, 0x25, 0x35, 0x00, 0x21, 0x31, 0x2D, 0x3D, 0x39, 0x00, 0x00};
uint8_t v_eor[] = {0x49, 0x45, 0x55, 0x00, 0x41, 0x51, 0x4D, 0x5D, 0x59, 0x00, 0x00};
uint8_t v_adc[] = {0x69, 0x65, 0x75, 0x00, 0x61, 0x71, 0x6D, 0x7D, 0x79, 0x00, 0x00};
uint8_t v_sbc[] = {0xe9, 0xe5, 0xf5, 0x00, 0xe1, 0xf1, 0xeD, 0xfD, 0xf9, 0x00, 0x00};
uint8_t v_cmp[] = {0xc9, 0xc5, 0xd5, 0x00, 0xc1, 0xd1, 0xcd, 0xdd, 0xd9, 0x00, 0x00};
uint8_t v_cpx[] = {0xe0, 0xe4, 0x00, 0x00, 0x00, 0x00, 0xec, 0x00, 0x00, 0x00, 0x00};
uint8_t v_cpy[] = {0xc0, 0xc4, 0x00, 0x00, 0x00, 0x00, 0xcc, 0x00, 0x00, 0x00, 0x00};
uint8_t v_dec[] = {0x00, 0xc6, 0xd6, 0x00, 0x00, 0x00, 0xce, 0xde, 0x00, 0x00, 0x00};
uint8_t v_inc[] = {0x00, 0xe6, 0xf6, 0x00, 0x00, 0x00, 0xee, 0xfe, 0x00, 0x00, 0x00};
uint8_t v_asl[] = {0x00, 0x06, 0x16, 0x00, 0x00, 0x00, 0x0e, 0x1e, 0x00, 0x00, 0x00};
uint8_t v_rol[] = {0x00, 0x26, 0x36, 0x00, 0x00, 0x00, 0x2e, 0x3e, 0x00, 0x00, 0x00};
uint8_t v_lsr[] = {0x00, 0x46, 0x56, 0x00, 0x00, 0x00, 0x4e, 0x5e, 0x00, 0x00, 0x00};
uint8_t v_ror[] = {0x00, 0x66, 0x76, 0x00, 0x00, 0x00, 0x6e, 0x7e, 0x00, 0x00, 0x00};
uint8_t v_lda[] = {0xa9, 0xa5, 0xb5, 0x00, 0xa1, 0xb1, 0xad, 0xbd, 0xb9, 0x00, 0x00};
uint8_t v_sta[] = {0x00, 0x85, 0x95, 0x00, 0x81, 0x91, 0x8d, 0x9d, 0x99, 0x00, 0x00};
uint8_t v_ldx[] = {0xa2, 0xa6, 0x00, 0xb6, 0x00, 0x00, 0xae, 0x00, 0xbe, 0x00, 0x00};
uint8_t v_stx[] = {0x00, 0x86, 0x00, 0x96, 0x00, 0x00, 0x8e, 0x00, 0x00, 0x00, 0x00};
uint8_t v_ldy[] = {0xa0, 0xa4, 0xb4, 0x00, 0x00, 0x00, 0xac, 0xbc, 0x00, 0x00, 0x00};
uint8_t v_sty[] = {0x00, 0x84, 0x94, 0x00, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00};

bool exec_ora(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_ora))
    {
        *res = a |= op;
        return 1;
    }
    return 0;
}

bool exec_and(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_and))
    {
        *res = a &= op;
        return 1;
    }
    return 0;
}

bool exec_eor(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_eor))
    {
        *res = a ^= op;
        return 1;
    }
    return 0;
}

int8_t setCV(int8_t a, int8_t b, bool sub)
{
    int8_t res = a + (sub ? ~b : b) + fC;
    if (sub)
        b = -b;
    uint8_t ua = a, ub = b, ur = res;
    fC = fC ? (ur <= ua) : (ur < ua);
    fV = (ua ^ ur) & (ub ^ ur) & (1 << 7);
    return res;
}

bool exec_adc(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_adc))
    {
        *res = a = setCV(a, op, 0);
        return 1;
    }
    return 0;
}

bool exec_sbc(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_sbc))
    {
        *res = a = setCV(a, op, 1);
        return 1;
    }
    return 0;
}

bool exec_cmp(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_cmp))
    {
        *res = a - op;
        fC = (uint8_t)a >= (uint8_t)op;
        return 1;
    }
    return 0;
}

bool exec_cpx(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_cpx))
    {
        *res = x - op;
        fC = (uint8_t)x >= (uint8_t)op;
        return 1;
    }
    return 0;
}

bool exec_cpy(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_cpy))
    {
        *res = y - op;
        fC = (uint8_t)y >= (uint8_t)op;
        return 1;
    }
    return 0;
}

bool exec_dec(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    uint16_t op_addr = 0;
    if (get_operand(&op, &op_addr, ins, v_dec))
    {
        mem_write(op_addr, *res = op - 1);
        return 1;
    }
    return 0;
}

bool exec_inc(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    uint16_t op_addr = 0;
    if (get_operand(&op, &op_addr, ins, v_inc))
    {
        mem_write(op_addr, *res = op + 1);
        return 1;
    }
    return 0;
}

bool exec_asl(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    uint16_t op_addr = 0;
    if (get_operand(&op, &op_addr, ins, v_asl))
    {
        fC = op >> 7;
        mem_write(op_addr, *res = (uint8_t)op << 1);
        return 1;
    }
    return 0;
}

bool exec_rol(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    uint16_t op_addr = 0;
    if (get_operand(&op, &op_addr, ins, v_rol))
    {
        bool c_in = fC;
        fC = op >> 7;
        mem_write(op_addr, *res = ((uint8_t)op << 1) | (uint8_t)c_in);
        return 1;
    }
    return 0;
}

bool exec_lsr(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    uint16_t op_addr = 0;
    if (get_operand(&op, &op_addr, ins, v_lsr))
    {
        fC = op & 1;
        mem_write(op_addr, *res = (uint8_t)op >> 1);
        return 1;
    }
    return 0;
}

bool exec_ror(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    uint16_t op_addr = 0;
    if (get_operand(&op, &op_addr, ins, v_ror))
    {
        bool c_in = fC;
        fC = op & 1;
        mem_write(op_addr, *res = ((uint8_t)op >> 1) | ((uint8_t)c_in << 7));
        return 1;
    }
    return 0;
}

bool exec_lda(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_lda))
    {
        *res = a = op;
        return 1;
    }
    return 0;
}

bool exec_sta(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    uint16_t op_addr = 0;
    if (get_operand(&op, &op_addr, ins, v_sta))
    {
        mem_write(op_addr, a);
        return 1;
    }
    return 0;
}

bool exec_ldx(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_ldx))
    {
        *res = x = op;
        return 1;
    }
    return 0;
}

bool exec_stx(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    uint16_t op_addr = 0;
    if (get_operand(&op, &op_addr, ins, v_stx))
    {
        mem_write(op_addr, x);
        return 1;
    }
    return 0;
}

bool exec_ldy(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    if (get_operand(&op, &useless_addr, ins, v_ldy))
    {
        *res = y = op;
        return 1;
    }
    return 0;
}

bool exec_sty(uint8_t ins, int8_t *res)
{
    int8_t op = 0;
    uint16_t op_addr = 0;
    if (get_operand(&op, &op_addr, ins, v_sty))
    {
        mem_write(op_addr, y);
        return 1;
    }
    return 0;
}

void exec_ins()
{
    uint8_t ins = (uint8_t)mem_read(pc);
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
    {
        res = --x;
        pc++;
    }
    else if (ins == 0x88)
    {
        res = --y;
        pc++;
    }
    else if (exec_inc(ins, &res))
        ;
    else if (ins == 0xe8)
    {
        res = ++x;
        pc++;
    }
    else if (ins == 0xc8)
    {
        res = ++y;
        pc++;
    }
    else if (ins == 0x0a)
    {
        fC = (uint8_t)a >> 7;
        res = a = (uint8_t)a << 1;
        pc++;
    }
    else if (exec_asl(ins, &res))
        ;
    else if (ins == 0x2a)
    {
        bool c_in = fC;
        fC = (uint8_t)a >> 7;
        res = a = ((uint8_t)a << 1) | (uint8_t)c_in;
        pc++;
    }
    else if (exec_rol(ins, &res))
        ;
    else if (ins == 0x4a)
    {
        fC = (uint8_t)a & 1;
        res = a = (uint8_t)a >> 1;
        pc++;
    }
    else if (exec_lsr(ins, &res))
        ;
    else if (ins == 0x6a)
    {
        bool c_in = fC;
        fC = (uint8_t)a & 1;
        res = a = ((uint8_t)a >> 1) | ((uint8_t)c_in << 7);
        pc++;
    }
    else if (exec_ror(ins, &res))
        ;
    else if (exec_lda(ins, &res))
        ;
    else if (exec_sta(ins, &res))
        skipZN = 1;
    else if (exec_ldx(ins, &res))
        ;
    else if (exec_stx(ins, &res))
        skipZN = 1;
    else if (exec_ldy(ins, &res))
        ;
    else if (exec_sty(ins, &res))
        skipZN = 1;
    else if (ins == 0xaa)
    {
        res = x = a;
        pc++;
    }
    else if (ins == 0x8a)
    {
        res = a = x;
        pc++;
    }
    else if (ins == 0xa8)
    {
        res = y = a;
        pc++;
    }
    else if (ins == 0x98)
    {
        res = a = y;
        pc++;
    }
    else if (ins == 0xba)
    {
        res = x = stack_pointer;
        pc++;
    }
    else if (ins == 0x9a)
    {
        stack_pointer = x;
        skipZN = 1;
        pc++;
    }
    else if (ins == 0x68)
    {
        res = a = s_pop();
        pc++;
    }
    else if (ins == 0x48)
    {
        s_push(a);
        skipZN = 1;
        pc++;
    }
    else if (ins == 0x28)
    {
        uint8_t status = s_pop();
        fN = status & (1 << 7);
        fV = status & (1 << 6);
        fD = status & (1 << 3);
        fI = status & (1 << 2);
        fZ = status & (1 << 1);
        fC = status & (1 << 0);
        skipZN = 1;
        pc++;
    }
    else if (ins == 0x08)
    {
        s_push((fN << 7) | (fV << 6) | (1 << 5) | (fB << 4) | (fD << 3) | (fI << 2) | (fZ << 1) | (fC << 0));
        skipZN = 1;
        pc++;
    }
    else if (ins == 0x10)
    {
        if (!fN)
            pc += mem_read(pc + 1);
        pc += 2;
        skipZN = 1;
    }
    else if (ins == 0x30)
    {
        if (fN)
            pc += mem_read(pc + 1);
        pc += 2;
        skipZN = 1;
    }
    else if (ins == 0x50)
    {
        if (!fV)
            pc += mem_read(pc + 1);
        pc += 2;
        skipZN = 1;
    }
    else if (ins == 0x70)
    {
        if (fV)
            pc += mem_read(pc + 1);
        pc += 2;
        skipZN = 1;
    }
    else if (ins == 0x90)
    {
        if (!fC)
            pc += mem_read(pc + 1);
        pc += 2;
        skipZN = 1;
    }
    else if (ins == 0xb0)
    {
        if (fC)
            pc += mem_read(pc + 1);
        pc += 2;
        skipZN = 1;
    }
    else if (ins == 0xd0)
    {
        if (!fZ)
            pc += mem_read(pc + 1);
        pc += 2;
        skipZN = 1;
    }
    else if (ins == 0xf0)
    {
        if (fZ)
            pc += mem_read(pc + 1);
        pc += 2;
        skipZN = 1;
    }
    else if (ins == 0x00)
    {
        s_push16(pc + 1);
        fB = 1;
        s_push((fN << 7) | (fV << 6) | (1 << 5) | (fB << 4) | (fD << 3) | (fI << 2) | (fZ << 1) | (fC << 0));
        fI = 1;
        pc = mem_read16(0xfffe);
        skipZN = 1;
    }
    else if (ins == 0x40)
    {
        uint8_t status = s_pop();
        fN = status & (1 << 7);
        fV = status & (1 << 6);
        // fB = status & (1 << 4);
        fD = status & (1 << 3);
        fI = status & (1 << 2);
        fZ = status & (1 << 1);
        fC = status & (1 << 0);
        pc = s_pop16();
        skipZN = 1;
    }
    else if (ins == 0x20)
    {
        s_push16(pc + 2);
        pc = mem_read16(pc + 1);
        skipZN = 1;
    }
    else if (ins == 0x60)
    {
        pc = s_pop16() + 1;
        skipZN = 1;
    }
    else if (ins == 0x4c)
    {
        pc = mem_read16(pc + 1);
        skipZN = 1;
    }
    else if (ins == 0x6c)
    {
        pc = mem_read16(mem_read16(pc + 1));
        skipZN = 1;
    }
    else if (ins == 0x24)
    {
        load_ins_buf(2);
        uint8_t op = (uint8_t)mem_read((uint8_t)ins_buf[1]);
        fZ = (a & op) == 0;
        fN = op & (1 << 7);
        fV = op & (1 << 6);
        skipZN = 1;
    }
    else if (ins == 0x2c)
    {
        load_ins_buf(3);
        uint8_t op = (uint8_t)mem_read(((uint16_t)(uint8_t)ins_buf[2] << 8) | (uint16_t)(uint8_t)ins_buf[1]);
        fZ = (a & op) == 0;
        fN = op & (1 << 7);
        fV = op & (1 << 6);
        skipZN = 1;
    }
    else if (ins == 0x18)
    {
        fC = 0;
        skipZN = 1;
        pc++;
    }
    else if (ins == 0x38)
    {
        fC = 1;
        skipZN = 1;
        pc++;
    }
    else if (ins == 0xd8)
    {
        fD = 0;
        skipZN = 1;
        pc++;
    }
    else if (ins == 0xf8)
    {
        fD = 1;
        skipZN = 1;
        pc++;
    }
    else if (ins == 0x58)
    {
        fI = 0;
        skipZN = 1;
        pc++;
    }
    else if (ins == 0x78)
    {
        fI = 1;
        skipZN = 1;
        pc++;
    }
    else if (ins == 0xb8)
    {
        fV = 0;
        skipZN = 1;
        pc++;
    }
    else if (ins == 0xea)
    {
        skipZN = 1;
        pc++;
    }
    else
    {
        reset();
    }

    if (!skipZN)
    {
        fZ = res == 0;
        fN = res & (1 << 7);
    }
}
