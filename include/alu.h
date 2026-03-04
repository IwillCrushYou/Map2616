#pragma once

#include <stdint.h>
#include "microprocessor.h"

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

/* ─── arithmetic operations ─────────────────── */

uint16_t alu_add(CPU *cpu, uint16_t a, uint16_t b);
uint16_t alu_sub(CPU *cpu, uint16_t a, uint16_t b);

/* ─── increment / decrement ─────────────────── */

uint16_t alu_inc(CPU *cpu, uint16_t a);
uint16_t alu_dec(CPU *cpu, uint16_t a);

/* ─── logical operations ────────────────────── */

uint16_t alu_and(CPU *cpu, uint16_t a, uint16_t b);
uint16_t alu_or(CPU *cpu, uint16_t a, uint16_t b);
uint16_t alu_xor(CPU *cpu, uint16_t a, uint16_t b);
uint16_t alu_not(CPU *cpu, uint16_t a);