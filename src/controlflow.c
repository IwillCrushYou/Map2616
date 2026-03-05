#include "controlflow.h"
/* ── Control Flow ───────────────────────────────────────────── */

void handle_jmp(CPU *cpu, Instruction *instr) {
    /* Unconditional jump to 12-bit address */
    cpu->PC = instr->immediate;
}

void handle_jz(CPU *cpu, Instruction *instr) {
    /* Jump if ZERO flag is set */
    if (flag_is_set(cpu, FLAG_ZERO))
        cpu->PC = instr->immediate;
}

void handle_jn(CPU *cpu, Instruction *instr) {
    /* Jump if NEGATIVE flag is set */
    if (flag_is_set(cpu, FLAG_NEGATIVE))
        cpu->PC = instr->immediate;
}