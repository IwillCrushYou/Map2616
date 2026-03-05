#include "memory.h"
#include "CPU.h"
#include "errors.h"
void mem_load_program(CPU *cpu, const uint16_t *words, size_t n)
{
    uint16_t addr = PROG_START;

    for(size_t i = 0; i < n; i++){
        mem_write_word(cpu, addr, words[i]);
        addr += 2;
    }
}

/* ═══════════════════════════════════════════════════════════════
 * MEMORY HELPERS
 * ═══════════════════════════════════════════════════════════════ */

/* Read a 16-bit value from memory (little-endian: low byte first) */
uint16_t mem_read_word(CPU *cpu, uint16_t addr) {
    if (addr + 1 >= MEM_SIZE) {
        cpu->halted = 1;
        cpu->error  = ERR_BAD_MEMORY;
        return 0;
    }
    return (uint16_t)(cpu->memory[addr] | (cpu->memory[addr + 1] << 8));
}

/* Write a 16-bit value to memory (little-endian: low byte first) */
void mem_write_word(CPU *cpu, uint16_t addr, uint16_t value) {
    if (addr + 1 >= MEM_SIZE) {
        cpu->halted = 1;
        cpu->error  = ERR_BAD_MEMORY;
        return;
    }
    cpu->memory[addr]     = (uint8_t)(value & 0x00FF);
    cpu->memory[addr + 1] = (uint8_t)(value >> 8);
}

uint8_t mem_read_byte(CPU *cpu, uint16_t addr){
    if(addr>=MEM_SIZE){
        cpu->halted = 1;
        cpu->error = ERR_BAD_MEMORY;
        return 0;
    }
    return (uint8_t) (cpu->memory[addr]);
}

void mem_write_byte(CPU *cpu, uint16_t addr, uint8_t val){
    if(addr >= MEM_SIZE){
        cpu->halted = 1;
        cpu->error = ERR_BAD_MEMORY;
        return ;
    }
    cpu->memory[addr] = (uint8_t) (val & 0x00FF);
}

/* ── Memory ─────────────────────────────────────────────────── */

void handle_load(CPU *cpu, Instruction *instr) {
    /* LOAD dst, addr  →  dst = memory[addr] */
    uint16_t value = mem_read_word(cpu, instr->immediate);
    if (cpu->halted) return;                /* bad memory read  */
    cpu->regs[instr->dst] = value;
}

void handle_stor(CPU *cpu, Instruction *instr) {
    /* STOR dst, addr  →  memory[addr] = dst */
    mem_write_word(cpu, instr->immediate, cpu->regs[instr->dst]);
}

void handle_mov(CPU *cpu, Instruction *instr) {
    /* MOV dst, imm8  →  dst = immediate value */
    cpu->regs[instr->dst] = instr->immediate;
}