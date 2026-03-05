#pragma once
#include "CPU.h"
#include "alu.h"
void handle_jmp(CPU * cpu, Instruction * instr);
void handle_jz(CPU *cpu, Instruction *instr);
void handle_jn(CPU *cpu, Instruction *instr);
