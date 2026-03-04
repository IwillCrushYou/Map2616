#pragma once

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