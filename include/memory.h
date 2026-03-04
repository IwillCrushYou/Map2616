#pragma once

#include "microprocessor.h"

/* ─── byte access ────────────────────────────── */
uint8_t  mem_read_byte (const CPU *cpu, uint16_t addr);
void     mem_write_byte(CPU *cpu, uint16_t addr, uint8_t  val);

/* ─── word access (big-endian) ───────────────── */
uint16_t mem_read_word (const CPU *cpu, uint16_t addr);
void     mem_write_word(CPU *cpu, uint16_t addr, uint16_t val);

/* ─── program loader ─────────────────────────── */
void     mem_load_program(CPU *cpu, const uint16_t *words, size_t n);

