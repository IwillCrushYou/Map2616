#pragma once
#include <cstdint>

// 15        12 11       8 7        4 3        0
// [ OPCODE 4b ][ DST REG ][ SRC REG ][ IMMEDIATE ]

// OR for immediate-heavy instructions:
// [ OPCODE 4b ][ DST REG ][   IMMEDIATE 8 bits   ]

#define OPCODE_MASK    0xF000
#define OPCODE_SHIFT   12

#define DST_MASK       0x0F00
#define DST_SHIFT      8

#define SRC_MASK       0x00F0
#define SRC_SHIFT      4

#define IMM4_MASK      0x000F   // 4-bit immediate
#define IMM8_MASK      0x00FF   // 8-bit immediate (when src+imm combined)

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
} Opcode;

typedef enum {
    ADDR_REGISTER,      // operand is a register
    ADDR_IMMEDIATE,     // operand is a literal value
    ADDR_DIRECT,        // operand is a memory address
    ADDR_INDIRECT,      // operand is a register holding an address
} AddrMode;

typedef struct {
    Opcode   opcode;
    uint8_t  dst;        // destination register index
    uint8_t  src;        // source register index
    uint8_t  immediate;  // immediate value
    AddrMode mode;
} Instruction;

#define FLAG_ZERO     (1 << 0)
#define FLAG_NEGATIVE (1 << 1)
#define FLAG_CARRY    (1 << 2)
#define FLAG_OVERFLOW (1 << 3)