# Map2616 Microprocessor Emulator

A custom **16-bit microprocessor emulator and assembler written in C**, capable of executing assembly programs through a simulated CPU architecture.

The emulator can run:

- as a **native Linux program**
- as **WebAssembly (WASM)** in Node.js
- inside a **browser using a React frontend**

This project demonstrates **systems programming, emulator design, assembly parsing, and WebAssembly deployment**.

---

# Features

## Custom 16-bit CPU Emulator

The emulator simulates a simplified microprocessor including:

- CPU registers
- memory simulation
- instruction decoding
- arithmetic logic unit (ALU)
- control flow instructions
- execution tracing

Assembly programs are translated into machine instructions and executed by the emulator.

---

## Built-in Assembler

A custom assembler converts assembly source code into executable instructions.

Capabilities include:

- label handling
- operand parsing
- syntax validation
- machine instruction generation

This allows developers to write assembly programs directly without external tools.

---

## Modular Emulator Architecture

The emulator is implemented using modular system components:

- **Registers module** – CPU register file
- **Memory module** – simulated RAM
- **ALU module** – arithmetic and logical operations
- **Decoder module** – instruction decoding
- **Control module** – execution flow and program counter

This structure improves maintainability and makes the emulator easier to extend.

---

## WebAssembly Deployment

The emulator can be compiled to **WebAssembly using Emscripten**.

This allows the same C codebase to run:

- as a native binary
- inside Node.js
- directly inside a browser

---

## React Frontend Interface

A React frontend provides a browser-based interface where users can:

- write assembly programs
- run them inside the emulator
- view execution output

The frontend communicates with the WebAssembly module using the **Emscripten virtual filesystem**.

---

# Project Structure

Map2616/
│
├── src/ # emulator implementation
├── include/ # header files
├── frontend/ # React frontend
├── build/ # native build
├── build-wasm/ # WebAssembly build
├── CMakeLists.txt
└── README.md


---

# Running the Project

## Native Build (Linux)

```bash
git clone https://github.com/IwillCrushYou/Map2616
cd Map2616

mkdir build
cd build

cmake ..
make

./main /path/to/program.txt/file
