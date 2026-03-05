#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "assembler.h"
#include "memory.h"

static int parse_register(char *token)
{
    if(token[0] != 'R') return -1;
    return atoi(&token[1]);
}

static uint8_t parse_opcode(char *token)
{
    if(strcmp(token,"ADD")==0) return OP_ADD;
    if(strcmp(token,"SUB")==0) return OP_SUB;
    if(strcmp(token,"MUL")==0) return OP_MUL;
    if(strcmp(token,"DIV")==0) return OP_DIV;

    if(strcmp(token,"AND")==0) return OP_AND;
    if(strcmp(token,"OR")==0)  return OP_OR;
    if(strcmp(token,"XOR")==0) return OP_XOR;
    if(strcmp(token,"NOT")==0) return OP_NOT;

    if(strcmp(token,"LOAD")==0) return OP_LOAD;
    if(strcmp(token,"STOR")==0) return OP_STOR;
    if(strcmp(token,"MOV")==0)  return OP_MOV;

    if(strcmp(token,"JMP")==0) return OP_JMP;
    if(strcmp(token,"JZ")==0)  return OP_JZ;
    if(strcmp(token,"JN")==0)  return OP_JN;

    if(strcmp(token,"NOP")==0) return OP_NOP;
    if(strcmp(token,"HALT")==0) return OP_HALT;

    return 255;
}

int load_program_text(CPU *cpu, const char *filename)
{
    FILE *f = fopen(filename,"r");

    if(!f){
        perror("program file");
        return -1;
    }

    char line[256];

    uint16_t addr = PROG_START;

    while(fgets(line,sizeof(line),f))
    {
        char *opcode = strtok(line," \t\n");
        if(!opcode) continue;

        uint8_t op = parse_opcode(opcode);

        if(op == OP_HALT || op == OP_NOP)
        {
            uint16_t instr = cpu_encode(op,0,0,0);
            mem_write_word(cpu,addr,instr);
            addr += 2;
            continue;
        }

        char *arg1 = strtok(NULL," \t\n");
        char *arg2 = strtok(NULL," \t\n");

        int dst = 0;
        int src = 0;
        int imm = 0;

        if(arg1) dst = parse_register(arg1);

        if(arg2)
        {
            if(arg2[0]=='R')
                src = parse_register(arg2);
            else
                imm = atoi(arg2);
        }

        uint16_t instr = cpu_encode(op,dst,src,imm);

        mem_write_word(cpu,addr,instr);

        addr += 2;
    }

    fclose(f);

    return 0;
}