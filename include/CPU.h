#pragma once
#include <isa.h>
#include <registers.h>
/* ─── sizing ─────────────────────────────────── */
#define MEM_SIZE    65536u
#define MAX_CYCLES  10000u
#define PROG_START  0x0000u

/* ─── CPU struct ─────────────────────────────── */
typedef struct {
    uint32_t cycles;             /* instructions executed      */
    uint16_t regs[NUM_REGS];     /* R0–R15 registers */
    uint16_t PC;                 /* instruction pointer        */
    uint16_t FLAGS;              /* Z[0]  C[1]  S[2]           */
    uint8_t  memory[MEM_SIZE];   /* 64 KB flat RAM             */
    uint8_t  halted;             /* 1 after HLT / bad opcode   */
    uint8_t  last_opcode;        /* opcode of last instruction */
    uint8_t  error;              /* 0=ok 1=bad_op 2=div0 3=bad_mem 4=max*/
    Instruction last_instr;      /* last decoded instruction            */
} CPU;