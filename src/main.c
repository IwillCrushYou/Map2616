#include <stdio.h>
#include "microprocessor.h"
#include "assembler.h"

int main(int argc,char **argv)
{
    if(argc < 2)
    {
        printf("Usage: %s program.txt\n",argv[0]);
        return 1;
    }

    CPU cpu;

    cpu_init(&cpu);

    load_program_text(&cpu,argv[1]);

    cpu_run(&cpu,1);

    return 0;
}
