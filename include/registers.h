#pragma once

#include <stdint.h>

/* ─── register configuration ────────────────── */
#define NUM_REGS 16

/* ─── register file ─────────────────────────── */

typedef struct {
    uint16_t r[NUM_REGS];   /* R0–R15 general purpose registers */
} Registers;


/* ─── register access ───────────────────────── */

static inline uint16_t reg_read(const Registers *regs, uint8_t index){
    return regs->r[index & 0x0F];
}
static inline void reg_write(Registers *regs, uint8_t index, uint16_t value){
    regs->r[index & 0x0F] = value;
}

/* ─── register reset ────────────────────────── */

static inline void reg_reset(Registers *regs){
    for (int i = 0; i < NUM_REGS; i++)
        regs->r[i] = 0;
}