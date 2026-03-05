#include <stdio.h>
#include <string.h>
#include "microprocessor.h"
#include "decoder.h"
#include "controlflow.h"

/* ═══════════════════════════════════════════════════════════════
 * OPCODE NAME TABLE  (for verbose output)
 * ═══════════════════════════════════════════════════════════════ */
static const char *opcode_names[16] = {
    "ADD", "SUB", "MUL", "DIV",
    "AND", "OR",  "XOR", "NOT",
    "LOAD","STOR","MOV",
    "JMP", "JZ",  "JN",
    "NOP", "HALT"
};

const char *cpu_opcode_name(uint8_t opcode) {
    if (opcode > 0xF) return "???";
    return opcode_names[opcode];
}

uint16_t cpu_encode(uint8_t opcode, uint8_t dst, uint8_t src, uint8_t imm)
{
    uint16_t instr = 0;

    instr |= (opcode & 0xF) << 12;
    instr |= (dst & 0xF) << 8;
    instr |= (src & 0xF) << 4;
    instr |= (imm & 0xF);

    return instr;
}

/* ═══════════════════════════════════════════════════════════════
 * VERBOSE PRINT  (called per step when verbose=1)
 * ═══════════════════════════════════════════════════════════════ */
static void print_step(CPU *cpu, uint16_t raw, Instruction *instr) {
    printf("[%04u] PC=%04X  raw=%04X  %-4s  dst=R%X src=R%X imm=%04X  "
           "FLAGS=%c%c%c%c\n",
           cpu->cycles,
           cpu->PC - 2,                         /* PC before increment */
           raw,
           cpu_opcode_name(instr->opcode),
           instr->dst,
           instr->src,
           instr->immediate,
           flag_is_set(cpu, FLAG_ZERO)     ? 'Z' : '-',
           flag_is_set(cpu, FLAG_CARRY)    ? 'C' : '-',
           flag_is_set(cpu, FLAG_NEGATIVE) ? 'N' : '-',
           flag_is_set(cpu, FLAG_OVERFLOW) ? 'V' : '-');
}

/* ═══════════════════════════════════════════════════════════════
 * LIFECYCLE
 * ═══════════════════════════════════════════════════════════════ */

void cpu_init(CPU *cpu) {
    memset(cpu, 0, sizeof(CPU));    /* zero everything including memory */
    cpu->PC = PROG_START;
}

void cpu_reset(CPU *cpu) {
    /* clear registers and state but preserve memory (program stays loaded) */
    memset(cpu->regs, 0, sizeof(cpu->regs));
    cpu->PC     = PROG_START;
    cpu->FLAGS  = 0;
    cpu->halted = 0;
    cpu->error  = ERR_NONE;
    cpu->cycles = 0;
}

void cpu_load(CPU *cpu, uint16_t addr, uint8_t *program, uint16_t size) {
    if (addr + size > MEM_SIZE) size = MEM_SIZE - addr;  /* clamp to fit */
    memcpy(&cpu->memory[addr], program, size);
}

/* ═══════════════════════════════════════════════════════════════
 * FETCH – DECODE – EXECUTE  (single step)
 * Returns: 1 = continue running,  0 = halted
 * ═══════════════════════════════════════════════════════════════ */
int cpu_step(CPU *cpu) {
    if (cpu->halted) return 0;

    /* ── FETCH ────────────────────────────────────────────────── */
    uint16_t raw = mem_read_word(cpu, cpu->PC);
    if (cpu->halted) return 0;          /* bad memory read halted us */
    cpu->PC += 2;                       /* advance past this instruction */

    /* ── DECODE ───────────────────────────────────────────────── */
    Instruction instr = decode(raw);
    cpu->last_instr   = instr;

    if (!instr.valid) {
        cpu->halted = 1;
        cpu->error  = ERR_BAD_OPCODE;
        return 0;
    }

    /* ── EXECUTE ──────────────────────────────────────────────── */
    switch (instr.opcode) {
        case OP_ADD:  handle_add (cpu, &instr); break;
        case OP_SUB:  handle_sub (cpu, &instr); break;
        case OP_MUL:  handle_mul (cpu, &instr); break;
        case OP_DIV:  handle_div (cpu, &instr); break;

        case OP_AND:  handle_and (cpu, &instr); break;
        case OP_OR:   handle_or  (cpu, &instr); break;
        case OP_XOR:  handle_xor (cpu, &instr); break;
        case OP_NOT:  handle_not (cpu, &instr); break;

        case OP_LOAD: handle_load(cpu, &instr); break;
        case OP_STOR: handle_stor(cpu, &instr); break;
        case OP_MOV:  handle_mov (cpu, &instr); break;

        case OP_JMP:  handle_jmp (cpu, &instr); break;
        case OP_JZ:   handle_jz  (cpu, &instr); break;
        case OP_JN:   handle_jn  (cpu, &instr); break;

        case OP_NOP:  /* intentionally do nothing */  break;

        case OP_HALT:
            cpu->halted = 1;
            cpu->error  = ERR_NONE;
            return 0;
    }

    cpu->cycles++;
    return !cpu->halted;
}

/* ═══════════════════════════════════════════════════════════════
 * RUN LOOP
 * Runs until HALT, error, or MAX_CYCLES exceeded.
 * ═══════════════════════════════════════════════════════════════ */
void cpu_run(CPU *cpu, int verbose) {
    while (!cpu->halted) {

        if (cpu->cycles >= MAX_CYCLES) {
            cpu->halted = 1;
            cpu->error  = ERR_MAX_CYCLES;
            fprintf(stderr, "ERROR: MAX_CYCLES (%u) exceeded — possible infinite loop\n",
                    MAX_CYCLES);
            break;
        }

        /* snapshot PC and raw before step advances them (for printing) */
        uint16_t pre_pc  = cpu->PC;
        uint16_t raw     = mem_read_word(cpu, pre_pc);

        int keep_running = cpu_step(cpu);

        if (verbose)
            print_step(cpu, raw, &cpu->last_instr);

        if (!keep_running) break;
    }

    if (verbose) {
        printf("\n── CPU halted ──────────────────────────────\n");
        printf("Cycles  : %u\n",   cpu->cycles);
        printf("PC      : 0x%04X\n", cpu->PC);
        printf("Error   : %u\n",   cpu->error);
        printf("Registers:\n");
        for (int i = 0; i < NUM_REGS; i++) {
            printf("  R%X = 0x%04X (%u)\n", i, cpu->regs[i], cpu->regs[i]);
        }
    }
}