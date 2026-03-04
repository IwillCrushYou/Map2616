#pragma once

/*
 * Every ALU function:
 *   - receives operand(s) by value
 *   - updates cpu->FLAGS
 *   - returns the 16-bit result
 *
 * ADD / SUB set Z, C, S.
 * INC / DEC set Z, S; Carry is preserved (8086 convention).
 */


