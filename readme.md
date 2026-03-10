# Microprocessor Emulator

## To run this project

### for linux

```bash
git clone https://github.com/IwillCrushYou/Map2616
cd Map2616
mkdir build
cd build
cmake ..
make 
./main /path/to/program.txt/file
```

## Assembler syntax

- One instruction per line.
- Case-insensitive opcodes/registers (for example: `mov r1 20`, `ADD R1 R2`).
- Operands may be separated by spaces or commas.
- Supported comments: `#`, `;`, and `//`.
- Numbers may be decimal or hex (`0x...`).
- Labels are supported using `label:` and can be used as jump/immediate targets.

Example:

```text
start:
MOV R1, 1
JMP end
MOV R1, 2   ; skipped
end: HALT
```
