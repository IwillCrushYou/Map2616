#pragma once
#include "isa.h"

/*
 * decode()
 * Takes a raw 16-bit instruction word.
 * Returns a fully populated Instruction struct.
 * If the opcode is unrecognized, returned struct has valid = 0.
 */
Instruction decode(uint16_t raw);

/*
 * get_format()
 * Returns the instruction format for a given opcode.
 * Useful if you need to inspect format before full decode.
 */
InstrFormat get_format(Opcode opcode);