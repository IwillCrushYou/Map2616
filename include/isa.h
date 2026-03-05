#pragma once
#include <stdint.h>

/*
 * 16-bit Instruction Format:
 *
 * REGISTER FORMAT:
 * 15      12 11      8 7       4 3       0
 * [ OPCODE ] [  DST  ] [  SRC  ] [ IMM4  ]
 *
 * IMMEDIATE FORMAT:
 * 15      12 11      8 7                 0
 * [ OPCODE ] [  DST  ] [   IMMEDIATE 8b ]
 *
 * JUMP FORMAT:
 * 15      12 11                          0
 * [ OPCODE ] [       ADDRESS 12b        ]
 *
 * IMPLIED FORMAT (NOP, HALT):
 * 15      12 11                          0
 * [ OPCODE ] [         unused           ]
 */

#define OPCODE_MASK    0xF000
#define OPCODE_SHIFT   12

#define DST_MASK       0x0F00
#define DST_SHIFT      8

#define SRC_MASK       0x00F0
#define SRC_SHIFT      4

#define IMM4_MASK      0x000F   // 4-bit immediate
#define IMM8_MASK      0x00FF   // 8-bit immediate (when src+imm combined)
#define IMM12_MASK     0x0FFF   // 12-bit 

typedef enum {
    /* Arithmetic */
    OP_ADD  = 0x0,
    OP_SUB  = 0x1,
    OP_MUL  = 0x2,
    OP_DIV  = 0x3,

    /* Logic */
    OP_AND  = 0x4,
    OP_OR   = 0x5,
    OP_XOR  = 0x6,
    OP_NOT  = 0x7,

    /* Load / Store */
    OP_LOAD = 0x8,
    OP_STOR = 0x9,
    OP_MOV  = 0xA,

    /* Control Flow */
    OP_JMP  = 0xB,
    OP_JZ   = 0xC,   // jump if zero
    OP_JN   = 0xD,   // jump if negative

    /* Misc */
    OP_NOP  = 0xE,
    OP_HALT = 0xF,

    OP_INC = 0x10,
    OP_DEC = 0x11,

} Opcode;

typedef enum {
    ADDR_REGISTER,      // operand is a register
    ADDR_IMMEDIATE,     // operand is a literal value
    ADDR_DIRECT,        // operand is a memory address
    ADDR_INDIRECT,      // operand is a register holding an address
    ADDR_IMPLIED,
} AddrMode;

/* ── Instruction Formats ────────────────────────────── */
typedef enum {
    FMT_REGISTER,       /* dst, src, imm4                         */
    FMT_IMMEDIATE,      /* dst, imm8                              */
    FMT_JUMP,           /* imm12 (address/offset)                 */
    FMT_IMPLIED,        /* no operands                            */
} InstrFormat;

typedef struct {
    Opcode   opcode;
    uint8_t  dst;        // destination register index
    uint8_t  src;        // source register index
    uint8_t  immediate;  // immediate value
    AddrMode mode;
    InstrFormat format;
    uint8_t valid;
} Instruction;

#define FLAG_ZERO     (1 << 0)
#define FLAG_NEGATIVE (1 << 1)
#define FLAG_CARRY    (1 << 2)
#define FLAG_OVERFLOW (1 << 3)