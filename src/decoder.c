#include "decoder.h"

/* ─────────────────────────────────────────────────────────────
 * Format lookup table
 * Maps every opcode to its instruction format.
 * This drives how operand bits are extracted.
 * ───────────────────────────────────────────────────────────── */
static const InstrFormat format_table[16] = {
    [OP_ADD]  = FMT_REGISTER,
    [OP_SUB]  = FMT_REGISTER,
    [OP_MUL]  = FMT_REGISTER,
    [OP_DIV]  = FMT_REGISTER,

    [OP_AND]  = FMT_REGISTER,
    [OP_OR]   = FMT_REGISTER,
    [OP_XOR]  = FMT_REGISTER,
    [OP_NOT]  = FMT_REGISTER,   /* only uses dst, src is ignored */

    [OP_LOAD] = FMT_IMMEDIATE,  /* dst, 8-bit memory address     */
    [OP_STOR] = FMT_IMMEDIATE,  /* dst, 8-bit memory address     */
    [OP_MOV]  = FMT_IMMEDIATE,  /* dst, 8-bit literal value      */

    [OP_JMP]  = FMT_JUMP,       /* 12-bit jump address           */
    [OP_JZ]   = FMT_JUMP,
    [OP_JN]   = FMT_JUMP,

    [OP_NOP]  = FMT_IMPLIED,
    [OP_HALT] = FMT_IMPLIED,
};

/* ─────────────────────────────────────────────────────────────
 * Addressing mode lookup table
 * Maps every opcode to its default addressing mode.
 * ───────────────────────────────────────────────────────────── */
static const AddrMode mode_table[16] = {
    [OP_ADD]  = ADDR_REGISTER,
    [OP_SUB]  = ADDR_REGISTER,
    [OP_MUL]  = ADDR_REGISTER,
    [OP_DIV]  = ADDR_REGISTER,

    [OP_AND]  = ADDR_REGISTER,
    [OP_OR]   = ADDR_REGISTER,
    [OP_XOR]  = ADDR_REGISTER,
    [OP_NOT]  = ADDR_REGISTER,

    [OP_LOAD] = ADDR_DIRECT,
    [OP_STOR] = ADDR_DIRECT,
    [OP_MOV]  = ADDR_IMMEDIATE,

    [OP_JMP]  = ADDR_IMMEDIATE,
    [OP_JZ]   = ADDR_IMMEDIATE,
    [OP_JN]   = ADDR_IMMEDIATE,

    [OP_NOP]  = ADDR_IMPLIED,   /* no addressing needed          */
    [OP_HALT] = ADDR_IMPLIED,
};

/* ─────────────────────────────────────────────────────────────
 * get_format()
 * ───────────────────────────────────────────────────────────── */
InstrFormat get_format(Opcode opcode) {
    return format_table[opcode & 0xF];
}

/* ─────────────────────────────────────────────────────────────
 * sign_extend_4bit()
 * Converts a 4-bit signed value to a full 16-bit signed value.
 * Example: 0b1110 (-2 in 4-bit) → 0xFFFE (-2 in 16-bit)
 * ───────────────────────────────────────────────────────────── */
static uint16_t sign_extend_4bit(uint8_t value) {
    if (value & 0x8) {              /* if bit 3 (sign bit) is set */
        return (uint16_t)(value | 0xFFF0);
    }
    return (uint16_t)value;
}

/* ─────────────────────────────────────────────────────────────
 * decode()
 * ───────────────────────────────────────────────────────────── */
Instruction decode(uint16_t raw) {
    Instruction instr = {0};        /* zero out all fields first  */

    /* ── Step 1: Extract opcode ──────────────────────────────── */
    uint8_t op = (raw & OPCODE_MASK) >> OPCODE_SHIFT;

    /* ── Step 2: Validate opcode ─────────────────────────────── */
    if (op > 0xF) {
        instr.valid = 0;
        return instr;
    }

    instr.opcode = (Opcode)op;
    instr.format = format_table[op];
    instr.mode   = mode_table[op];
    instr.valid  = 1;

    /* ── Step 3: Extract operands based on format ────────────── */
    switch (instr.format) {

        case FMT_REGISTER:
            /*
             * Bits: [15-12 opcode] [11-8 dst] [7-4 src] [3-0 imm4]
             */
            instr.dst       = (raw & DST_MASK) >> DST_SHIFT;
            instr.src       = (raw & SRC_MASK) >> SRC_SHIFT;
            instr.immediate = sign_extend_4bit(raw & IMM4_MASK);
            break;

        case FMT_IMMEDIATE:
            /*
             * Bits: [15-12 opcode] [11-8 dst] [7-0 imm8]
             * src bits are reused as upper half of immediate
             */
            instr.dst       = (raw & DST_MASK) >> DST_SHIFT;
            instr.src       = 0;
            instr.immediate = (raw & IMM8_MASK);
            break;

        case FMT_JUMP:
            /*
             * Bits: [15-12 opcode] [11-0 address]
             * dst and src bits are reused as part of address
             */
            instr.dst       = 0;
            instr.src       = 0;
            instr.immediate = (raw & IMM12_MASK);
            break;

        case FMT_IMPLIED:
            /*
             * No operands. All lower bits ignored.
             */
            instr.dst       = 0;
            instr.src       = 0;
            instr.immediate = 0;
            break;
    }

    return instr;
}