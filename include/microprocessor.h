#pragma once
#include <stdint.h>

/* ─── sizing ─────────────────────────────────── */
#define MEM_SIZE    65536u
#define MAX_CYCLES  10000u
#define PROG_START  0x0000u


/* ─── CPU struct ─────────────────────────────── */
typedef struct {
    uint32_t cycles;             /* instructions executed      */
    uint16_t AX;                 /* accumulator                */
    uint16_t BX;                 /* general-purpose            */
    uint16_t IP;                 /* instruction pointer        */
    uint8_t  FLAGS;              /* Z[0]  C[1]  S[2]           */
    uint8_t  memory[MEM_SIZE];   /* 64 KB flat RAM             */
    uint8_t  halted;             /* 1 after HLT / bad opcode   */
    uint8_t  last_opcode;        /* opcode of last instruction */
} CPU;

/* ─── lifecycle ──────────────────────────────── */
void     cpu_init (CPU *cpu);          /* zero everything            */
void     cpu_reset(CPU *cpu);          /* clear regs, keep memory    */

/* ─── fetch-decode-execute ───────────────────── */
uint16_t cpu_encode  (uint8_t opcode, uint8_t imm8);
int      cpu_step    (CPU *cpu);       /* 1=continue, 0=halt         */
void     cpu_run     (CPU *cpu, int verbose);
const char *cpu_opcode_name(uint8_t opcode);
