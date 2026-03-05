#pragma once
#include "errors.h"
#include "alu.h"
#include "memory.h"

/* ─── lifecycle ──────────────────────────────── */
void     cpu_init (CPU *cpu);          /* zero everything            */
void     cpu_reset(CPU *cpu);          /* clear regs, keep memory    */
void     cpu_load  (CPU *cpu, uint16_t addr, uint8_t *program, uint16_t size);


/* ─── fetch-decode-execute ───────────────────── */
uint16_t cpu_encode  (uint8_t opcode, uint8_t dst, uint8_t src, uint8_t imm);
int      cpu_step    (CPU *cpu);       /* 1=continue, 0=halt         */
void     cpu_run     (CPU *cpu, int verbose);
const char *cpu_opcode_name(uint8_t opcode);
