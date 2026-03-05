#include <stdio.h>
#include "microprocessor.h"

int main()
{
    CPU cpu;
    cpu_init(&cpu);

    uint16_t program[] = {
        cpu_encode(OP_MOV, 1, 0, 5),   // R1 = 5
        cpu_encode(OP_MOV, 2, 0, 3),   // R2 = 3
        cpu_encode(OP_ADD, 1, 2, 0),   // R1 = R1 + R2
        cpu_encode(OP_HALT,0,0,0)
    };

    mem_load_program(&cpu, program, 4);

    cpu_run(&cpu, 1);

    return 0;
}
