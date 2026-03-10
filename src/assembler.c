#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "assembler.h"
#include "memory.h"

typedef struct {
    int line_no;
    char text[256];
} SourceLine;

typedef struct {
    char name[64];
    uint16_t addr;
} Label;

static void strip_comments(char *line)
{
    char *p_hash = strchr(line, '#');
    char *p_semicolon = strchr(line, ';');
    char *p_slash = strstr(line, "//");
    char *cut = NULL;

    if (p_hash) cut = p_hash;
    if (p_semicolon && (!cut || p_semicolon < cut)) cut = p_semicolon;
    if (p_slash && (!cut || p_slash < cut)) cut = p_slash;

    if (cut) *cut = '\0';
}

static char *trim_whitespace(char *str)
{
    while (*str && isspace((unsigned char)*str)) str++;

    if (*str == '\0') return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return str;
}

static void uppercase_inplace(char *s)
{
    while (*s) {
        *s = (char)toupper((unsigned char)*s);
        s++;
    }
}

static int parse_register(const char *token, int *out_reg)
{
    if (!token || !out_reg) return -1;

    if (!(token[0] == 'R' || token[0] == 'r')) return -1;
    if (token[1] == '\0') return -1;

    char *end = NULL;
    long value = strtol(token + 1, &end, 10);
    if (*end != '\0') return -1;
    if (value < 0 || value >= NUM_REGS) return -1;

    *out_reg = (int)value;
    return 0;
}

static uint8_t parse_opcode(const char *token)
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

static InstrFormat opcode_format(uint8_t op)
{
    switch (op) {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_AND:
        case OP_OR:
        case OP_XOR:
        case OP_NOT:
            return FMT_REGISTER;
        case OP_LOAD:
        case OP_STOR:
        case OP_MOV:
            return FMT_IMMEDIATE;
        case OP_JMP:
        case OP_JZ:
        case OP_JN:
            return FMT_JUMP;
        case OP_NOP:
        case OP_HALT:
            return FMT_IMPLIED;
        default:
            return FMT_IMPLIED;
    }
}

static int parse_u16(const char *token, uint16_t *out)
{
    if (!token || !out) return -1;

    char *end = NULL;
    long value = strtol(token, &end, 0);
    if (*end != '\0') return -1;
    if (value < 0 || value > 0xFFFF) return -1;

    *out = (uint16_t)value;
    return 0;
}

static int find_label(const Label *labels, size_t n_labels, const char *name, uint16_t *addr)
{
    for (size_t i = 0; i < n_labels; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            if (addr) *addr = labels[i].addr;
            return 0;
        }
    }
    return -1;
}

static int add_label(Label **labels, size_t *count, size_t *capacity, const char *name, uint16_t addr, int line_no)
{
    if (find_label(*labels, *count, name, NULL) == 0) {
        fprintf(stderr, "Assembler error (line %d): duplicate label '%s'\n", line_no, name);
        return -1;
    }

    if (*count == *capacity) {
        size_t new_cap = (*capacity == 0) ? 16 : (*capacity * 2);
        Label *new_labels = (Label *)realloc(*labels, new_cap * sizeof(Label));
        if (!new_labels) {
            fprintf(stderr, "Assembler error: out of memory while storing labels\n");
            return -1;
        }
        *labels = new_labels;
        *capacity = new_cap;
    }

    strncpy((*labels)[*count].name, name, sizeof((*labels)[*count].name) - 1);
    (*labels)[*count].name[sizeof((*labels)[*count].name) - 1] = '\0';
    (*labels)[*count].addr = addr;
    (*count)++;

    return 0;
}

static int tokenize_line(char *line, char **tokens, int max_tokens)
{
    int count = 0;
    char *token = strtok(line, " \t,\r\n");

    while (token && count < max_tokens) {
        tokens[count++] = token;
        token = strtok(NULL, " \t,\r\n");
    }

    return count;
}

static int resolve_value(const char *token, const Label *labels, size_t n_labels, uint16_t *value)
{
    if (parse_u16(token, value) == 0) return 0;
    return find_label(labels, n_labels, token, value);
}

static int emit_instruction(
    CPU *cpu,
    uint16_t addr,
    int line_no,
    uint8_t op,
    char **operands,
    int operand_count,
    const Label *labels,
    size_t n_labels)
{
    InstrFormat format = opcode_format(op);
    uint16_t instr = 0;

    if (format == FMT_IMPLIED) {
        if (operand_count != 0) {
            fprintf(stderr, "Assembler error (line %d): '%s' takes no operands\n", line_no, cpu_opcode_name(op));
            return -1;
        }
        instr = ((uint16_t)(op & 0xF) << 12);
    } else if (format == FMT_JUMP) {
        if (operand_count != 1) {
            fprintf(stderr, "Assembler error (line %d): '%s' expects 1 operand\n", line_no, cpu_opcode_name(op));
            return -1;
        }

        uint16_t target = 0;
        if (resolve_value(operands[0], labels, n_labels, &target) != 0) {
            fprintf(stderr, "Assembler error (line %d): unknown label or invalid value '%s'\n", line_no, operands[0]);
            return -1;
        }
        if (target > IMM12_MASK) {
            fprintf(stderr, "Assembler error (line %d): jump target out of range (0..4095): %u\n", line_no, target);
            return -1;
        }

        instr = ((uint16_t)(op & 0xF) << 12) | (target & IMM12_MASK);
    } else if (format == FMT_IMMEDIATE) {
        if (operand_count != 2) {
            fprintf(stderr, "Assembler error (line %d): '%s' expects 2 operands (dst, imm/address)\n", line_no, cpu_opcode_name(op));
            return -1;
        }

        int dst = 0;
        if (parse_register(operands[0], &dst) != 0) {
            fprintf(stderr, "Assembler error (line %d): invalid destination register '%s'\n", line_no, operands[0]);
            return -1;
        }

        uint16_t value = 0;
        if (resolve_value(operands[1], labels, n_labels, &value) != 0) {
            fprintf(stderr, "Assembler error (line %d): unknown label or invalid value '%s'\n", line_no, operands[1]);
            return -1;
        }
        if (value > IMM8_MASK) {
            fprintf(stderr, "Assembler error (line %d): immediate/address out of range (0..255): %u\n", line_no, value);
            return -1;
        }

        instr = ((uint16_t)(op & 0xF) << 12)
              | ((uint16_t)(dst & 0xF) << 8)
              | (value & IMM8_MASK);
    } else {
        if (op == OP_NOT) {
            if (operand_count != 1) {
                fprintf(stderr, "Assembler error (line %d): 'NOT' expects 1 operand (dst)\n", line_no);
                return -1;
            }

            int dst = 0;
            if (parse_register(operands[0], &dst) != 0) {
                fprintf(stderr, "Assembler error (line %d): invalid destination register '%s'\n", line_no, operands[0]);
                return -1;
            }

            instr = cpu_encode(op, (uint8_t)dst, 0, 0);
        } else {
            if (operand_count != 2 && operand_count != 3) {
                fprintf(stderr, "Assembler error (line %d): '%s' expects 2 or 3 operands (dst, src[, imm4])\n", line_no, cpu_opcode_name(op));
                return -1;
            }

            int dst = 0;
            int src = 0;
            if (parse_register(operands[0], &dst) != 0) {
                fprintf(stderr, "Assembler error (line %d): invalid destination register '%s'\n", line_no, operands[0]);
                return -1;
            }
            if (parse_register(operands[1], &src) != 0) {
                fprintf(stderr, "Assembler error (line %d): invalid source register '%s'\n", line_no, operands[1]);
                return -1;
            }

            int imm4 = 0;
            if (operand_count == 3) {
                char *end = NULL;
                long raw = strtol(operands[2], &end, 0);
                if (*end != '\0' || raw < -8 || raw > 15) {
                    fprintf(stderr, "Assembler error (line %d): imm4 out of range (-8..15): '%s'\n", line_no, operands[2]);
                    return -1;
                }
                imm4 = (int)(raw & 0xF);
            }

            instr = cpu_encode(op, (uint8_t)dst, (uint8_t)src, (uint8_t)imm4);
        }
    }

    mem_write_word(cpu, addr, instr);
    return 0;
}

int load_program_text(CPU *cpu, const char *filename)
{
    FILE *f = fopen(filename, "r");

    if(!f){
        perror("program file");
        return -1;
    }

    SourceLine *lines = NULL;
    size_t line_count = 0;
    size_t line_capacity = 0;

    char raw[256];
    int line_no = 0;

    while (fgets(raw, sizeof(raw), f)) {
        line_no++;
        if (line_count == line_capacity) {
            size_t new_cap = (line_capacity == 0) ? 32 : (line_capacity * 2);
            SourceLine *new_lines = (SourceLine *)realloc(lines, new_cap * sizeof(SourceLine));
            if (!new_lines) {
                fprintf(stderr, "Assembler error: out of memory while reading source\n");
                free(lines);
                fclose(f);
                return -1;
            }
            lines = new_lines;
            line_capacity = new_cap;
        }

        lines[line_count].line_no = line_no;
        strncpy(lines[line_count].text, raw, sizeof(lines[line_count].text) - 1);
        lines[line_count].text[sizeof(lines[line_count].text) - 1] = '\0';
        line_count++;
    }

    fclose(f);

    Label *labels = NULL;
    size_t n_labels = 0;
    size_t labels_capacity = 0;

    uint16_t addr = PROG_START;

    for (size_t i = 0; i < line_count; i++) {
        char line[256];
        strncpy(line, lines[i].text, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';

        strip_comments(line);
        char *trimmed = trim_whitespace(line);
        if (*trimmed == '\0') continue;

        char *tokens[8] = {0};
        int n = tokenize_line(trimmed, tokens, 8);
        if (n == 0) continue;

        int idx = 0;
        size_t len0 = strlen(tokens[0]);
        if (len0 > 1 && tokens[0][len0 - 1] == ':') {
            tokens[0][len0 - 1] = '\0';
            uppercase_inplace(tokens[0]);
            if (add_label(&labels, &n_labels, &labels_capacity, tokens[0], addr, lines[i].line_no) != 0) {
                free(labels);
                free(lines);
                return -1;
            }
            idx = 1;
        }

        if (idx >= n) continue;

        uppercase_inplace(tokens[idx]);
        uint8_t op = parse_opcode(tokens[idx]);
        if (op == 255) {
            fprintf(stderr, "Assembler error (line %d): unknown opcode '%s'\n", lines[i].line_no, tokens[idx]);
            free(labels);
            free(lines);
            return -1;
        }

        if (addr > (uint16_t)(MEM_SIZE - 2)) {
            fprintf(stderr, "Assembler error (line %d): program exceeds memory size\n", lines[i].line_no);
            free(labels);
            free(lines);
            return -1;
        }
        addr += 2;
    }

    addr = PROG_START;

    for (size_t i = 0; i < line_count; i++) {
        char line[256];
        strncpy(line, lines[i].text, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';

        strip_comments(line);
        char *trimmed = trim_whitespace(line);
        if (*trimmed == '\0') continue;

        char *tokens[8] = {0};
        int n = tokenize_line(trimmed, tokens, 8);
        if (n == 0) continue;

        int idx = 0;
        size_t len0 = strlen(tokens[0]);
        if (len0 > 1 && tokens[0][len0 - 1] == ':') {
            tokens[0][len0 - 1] = '\0';
            idx = 1;
        }

        if (idx >= n) continue;

        uppercase_inplace(tokens[idx]);
        uint8_t op = parse_opcode(tokens[idx]);
        if (op == 255) {
            fprintf(stderr, "Assembler error (line %d): unknown opcode '%s'\n", lines[i].line_no, tokens[idx]);
            free(labels);
            free(lines);
            return -1;
        }

        int operand_count = n - idx - 1;
        char **operands = &tokens[idx + 1];

        for (int k = 0; k < operand_count; k++) {
            uppercase_inplace(operands[k]);
        }

        if (emit_instruction(cpu, addr, lines[i].line_no, op, operands, operand_count, labels, n_labels) != 0) {
            free(labels);
            free(lines);
            return -1;
        }

        addr += 2;
    }

    free(labels);
    free(lines);
    return 0;
}