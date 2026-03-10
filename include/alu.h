#pragma once
#include "CPU.h"

/*
 * Every ALU function:
 *   - receives operand(s) by value
 *   - updates cpu->FLAGS
 *   - returns the 16-bit result
 *
 * ADD / SUB set: ZERO, NEGATIVE, CARRY, OVERFLOW
 * INC / DEC set: ZERO, NEGATIVE
 * Carry is preserved (8086-style behavior).
 */

/* flag helpers */

static void flag_set(CPU *cpu, uint16_t flag);
static void flag_clear(CPU *cpu, uint16_t flag);
static void flags_zn(CPU *cpu, uint16_t result);

/* ─── arithmetic operations ─────────────────── */

uint16_t alu_add(CPU *cpu, uint16_t a, uint16_t b);
uint16_t alu_sub(CPU *cpu, uint16_t a, uint16_t b);
uint16_t alu_mul(CPU *cpu, uint16_t a, uint16_t b);
uint16_t alu_div(CPU *cpu, uint16_t a, uint16_t b);


/* ─── increment / decrement ─────────────────── */

uint16_t alu_inc(CPU *cpu, uint16_t a);
uint16_t alu_dec(CPU *cpu, uint16_t a);

/* ─── logical operations ────────────────────── */

uint16_t alu_and(CPU *cpu, uint16_t a, uint16_t b);
uint16_t alu_or(CPU *cpu, uint16_t a, uint16_t b);
uint16_t alu_xor(CPU *cpu, uint16_t a, uint16_t b);
uint16_t alu_not(CPU *cpu, uint16_t a);

void handle_add(CPU *cpu, Instruction *instr);
void handle_sub(CPU *cpu, Instruction *instr);
void handle_mul(CPU *cpu, Instruction *instr);
void handle_div(CPU *cpu, Instruction *instr);

void handle_and(CPU *cpu, Instruction *instr);
void handle_or (CPU *cpu, Instruction *instr);
void handle_xor(CPU *cpu, Instruction *instr);
void handle_not(CPU *cpu, Instruction *instr);

static inline int flag_is_set(const CPU *cpu, uint16_t flag){
    return (cpu->FLAGS & flag) != 0;
}