#pragma once
#include <stdint.h>
typedef struct {
    uint16_t result;
    uint8_t  flags;     /* bits set from FLAG_* defines in isa.h */
} ALUResult;

ALUResult alu_add(uint16_t a, uint16_t b);
ALUResult alu_sub(uint16_t a, uint16_t b);
ALUResult alu_mul(uint16_t a, uint16_t b);
ALUResult alu_div(uint16_t a, uint16_t b);
ALUResult alu_and(uint16_t a, uint16_t b);
ALUResult alu_or (uint16_t a, uint16_t b);
ALUResult alu_xor(uint16_t a, uint16_t b);
ALUResult alu_not(uint16_t a);             /* only one operand */
