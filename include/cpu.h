#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "memory.h"

extern uint16_t pc;
extern int8_t a, x, y;
extern bool fN, fV, fB, fD, fI, fZ, fC;
extern uint8_t stack_pointer;
extern int8_t ins_buf[3];

void reset();
void irq();
uint8_t s_pop();
void s_push(int8_t d);
uint16_t s_pop16();
void s_push16(uint16_t d);
void load_ins_buf(uint8_t len);
bool get_operand(int8_t *op, uint16_t *op_addr, uint8_t ins, uint8_t *variants);
bool exec_ora(uint8_t ins, int8_t *res);
bool exec_and(uint8_t ins, int8_t *res);
bool exec_eor(uint8_t ins, int8_t *res);
int8_t setCV(int8_t a, int8_t b, bool sub);
bool exec_adc(uint8_t ins, int8_t *res);
bool exec_sbc(uint8_t ins, int8_t *res);
bool exec_cmp(uint8_t ins, int8_t *res);
bool exec_cpx(uint8_t ins, int8_t *res);
bool exec_cpy(uint8_t ins, int8_t *res);
bool exec_dec(uint8_t ins, int8_t *res);
bool exec_inc(uint8_t ins, int8_t *res);
bool exec_asl(uint8_t ins, int8_t *res);
bool exec_rol(uint8_t ins, int8_t *res);
bool exec_lsr(uint8_t ins, int8_t *res);
bool exec_ror(uint8_t ins, int8_t *res);
bool exec_lda(uint8_t ins, int8_t *res);
bool exec_sta(uint8_t ins, int8_t *res);
bool exec_ldx(uint8_t ins, int8_t *res);
bool exec_stx(uint8_t ins, int8_t *res);
bool exec_ldy(uint8_t ins, int8_t *res);
bool exec_sty(uint8_t ins, int8_t *res);
void exec_ins();
