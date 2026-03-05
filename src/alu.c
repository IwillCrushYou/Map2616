#include "alu.h"

/* ═══════════════════════════════════════════════════════════════
 * INTERNAL FLAG HELPERS
 * ═══════════════════════════════════════════════════════════════ */

static void flag_set(CPU *cpu, uint8_t flag) {
    cpu->FLAGS |= flag;
}

static void flag_clear(CPU *cpu, uint8_t flag) {
    cpu->FLAGS &= ~flag;
}

/* Set ZERO and NEGATIVE based on result — called by every operation */
static void flags_zn(CPU *cpu, uint16_t result) {
    if (result == 0)
        flag_set(cpu, FLAG_ZERO);
    else
        flag_clear(cpu, FLAG_ZERO);

    if (result & 0x8000)
        flag_set(cpu, FLAG_NEGATIVE);
    else
        flag_clear(cpu, FLAG_NEGATIVE);
}


/* ═══════════════════════════════════════════════════════════════
 * ARITHMETIC
 * ═══════════════════════════════════════════════════════════════ */

uint16_t alu_add(CPU *cpu, uint16_t a, uint16_t b) {
    uint32_t full   = (uint32_t)a + (uint32_t)b;
    uint16_t result = (uint16_t)full;

    /* CARRY: result spilled past 16 bits */
    if (full > 0xFFFF)
        flag_set(cpu, FLAG_CARRY);
    else
        flag_clear(cpu, FLAG_CARRY);

    /*
     * OVERFLOW: signed result has wrong sign
     * Triggers when both inputs share a sign but the result differs.
     * Classic bitmask:  ~(a ^ b)  detects same-sign inputs.
     *                    (a ^ result) detects sign change in result.
     */
    if ((~(a ^ b)) & (a ^ result) & 0x8000)
        flag_set(cpu, FLAG_OVERFLOW);
    else
        flag_clear(cpu, FLAG_OVERFLOW);

    flags_zn(cpu, result);
    return result;
}

uint16_t alu_sub(CPU *cpu, uint16_t a, uint16_t b) {
    uint16_t result = a - b;

    /* CARRY = borrow: a was smaller than b (unsigned) */
    if (a < b)
        flag_set(cpu, FLAG_CARRY);
    else
        flag_clear(cpu, FLAG_CARRY);

    /*
     * OVERFLOW: signed result has wrong sign.
     * Triggers when inputs have different signs and result
     * has the same sign as b (the subtrahend), not a.
     * Bitmask: (a ^ b) detects different-sign inputs.
     *          (a ^ result) detects sign change in result.
     */
    if ((a ^ b) & (a ^ result) & 0x8000)
        flag_set(cpu, FLAG_OVERFLOW);
    else
        flag_clear(cpu, FLAG_OVERFLOW);

    flags_zn(cpu, result);
    return result;
}

uint16_t alu_mul(CPU *cpu, uint16_t a, uint16_t b) {
    uint32_t full   = (uint32_t)a * (uint32_t)b;
    uint16_t result = (uint16_t)(full & 0xFFFF);

    /* MUL does not produce CARRY or OVERFLOW in this ISA */
    flag_clear(cpu, FLAG_CARRY);
    flag_clear(cpu, FLAG_OVERFLOW);

    flags_zn(cpu, result);
    return result;
}

uint16_t alu_div(CPU *cpu, uint16_t a, uint16_t b) {
    /* Division by zero: halt the CPU */
    if (b == 0) {
        cpu->halted = 1;
        cpu->error  = 2;        /* ERR_DIV_ZERO */
        return 0;
    }

    uint16_t result = a / b;

    flag_clear(cpu, FLAG_CARRY);
    flag_clear(cpu, FLAG_OVERFLOW);

    flags_zn(cpu, result);
    return result;
}

/* ═══════════════════════════════════════════════════════════════
 * INCREMENT / DECREMENT
 * 8086-style: CARRY is preserved, not touched.
 * Only ZERO and NEGATIVE are updated.
 * ═══════════════════════════════════════════════════════════════ */

uint16_t alu_inc(CPU *cpu, uint16_t a) {
    uint16_t result = a + 1;
    /* CARRY intentionally not modified — preserved from before */
    flags_zn(cpu, result);
    return result;
}

uint16_t alu_dec(CPU *cpu, uint16_t a) {
    uint16_t result = a - 1;
    /* CARRY intentionally not modified — preserved from before */
    flags_zn(cpu, result);
    return result;
}

/* ═══════════════════════════════════════════════════════════════
 * LOGICAL
 * Logical ops always clear CARRY and OVERFLOW.
 * Only ZERO and NEGATIVE reflect the result.
 * ═══════════════════════════════════════════════════════════════ */

uint16_t alu_and(CPU *cpu, uint16_t a, uint16_t b) {
    uint16_t result = a & b;
    flag_clear(cpu, FLAG_CARRY);
    flag_clear(cpu, FLAG_OVERFLOW);
    flags_zn(cpu, result);
    return result;
}

uint16_t alu_or(CPU *cpu, uint16_t a, uint16_t b) {
    uint16_t result = a | b;
    flag_clear(cpu, FLAG_CARRY);
    flag_clear(cpu, FLAG_OVERFLOW);
    flags_zn(cpu, result);
    return result;
}

uint16_t alu_xor(CPU *cpu, uint16_t a, uint16_t b) {
    uint16_t result = a ^ b;
    flag_clear(cpu, FLAG_CARRY);
    flag_clear(cpu, FLAG_OVERFLOW);
    flags_zn(cpu, result);
    return result;
}

uint16_t alu_not(CPU *cpu, uint16_t a) {
    uint16_t result = ~a;
    flag_clear(cpu, FLAG_CARRY);
    flag_clear(cpu, FLAG_OVERFLOW);
    flags_zn(cpu, result);
    return result;
}

void handle_add(CPU *cpu, Instruction *instr)
{
    uint16_t a = cpu->regs[instr->dst];
    uint16_t b = cpu->regs[instr->src];

    cpu->regs[instr->dst] = alu_add(cpu, a, b);
}

void handle_sub(CPU *cpu, Instruction *instr)
{
    uint16_t a = cpu->regs[instr->dst];
    uint16_t b = cpu->regs[instr->src];

    cpu->regs[instr->dst] = alu_sub(cpu, a, b);
}

void handle_mul(CPU *cpu, Instruction *instr)
{
    uint16_t a = cpu->regs[instr->dst];
    uint16_t b = cpu->regs[instr->src];

    cpu->regs[instr->dst] = alu_mul(cpu, a, b);
}

void handle_div(CPU *cpu, Instruction *instr)
{
    uint16_t a = cpu->regs[instr->dst];
    uint16_t b = cpu->regs[instr->src];

    cpu->regs[instr->dst] = alu_div(cpu, a, b);
}

void handle_and(CPU *cpu, Instruction *instr)
{
    uint16_t a = cpu->regs[instr->dst];
    uint16_t b = cpu->regs[instr->src];

    cpu->regs[instr->dst] = alu_and(cpu, a, b);
}

void handle_or(CPU *cpu, Instruction *instr)
{
    uint16_t a = cpu->regs[instr->dst];
    uint16_t b = cpu->regs[instr->src];

    cpu->regs[instr->dst] = alu_or(cpu, a, b);
}

void handle_xor(CPU *cpu, Instruction *instr)
{
    uint16_t a = cpu->regs[instr->dst];
    uint16_t b = cpu->regs[instr->src];

    cpu->regs[instr->dst] = alu_xor(cpu, a, b);
}

void handle_not(CPU *cpu, Instruction *instr)
{
    uint16_t a = cpu->regs[instr->dst];

    cpu->regs[instr->dst] = alu_not(cpu, a);
}
