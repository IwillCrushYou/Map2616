# Map2616 — 16-bit Microprocessor Emulator

A custom **16-bit microprocessor emulator and assembler written in C**, capable of executing assembly programs through a fully simulated CPU architecture. The same C codebase runs natively on Linux, as WebAssembly in Node.js, and inside a browser via a React frontend.

---

## Table of Contents

- [Features](#features)
- [Architecture Overview](#architecture-overview)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Native Build (Linux)](#native-build-linux)
  - [WebAssembly Build](#webassembly-build)
  - [React Frontend](#react-frontend)
- [Assembly Language Reference](#assembly-language-reference)
- [Contributing](#contributing)
  - [Contributing on Linux](#contributing-on-linux)
  - [Contributing on Windows](#contributing-on-windows)
- [License](#license)

---

## Features

### Custom 16-bit CPU Emulator

Simulates a simplified but complete microprocessor including:

- CPU register file
- Simulated RAM (memory module)
- Instruction decoding pipeline
- Arithmetic Logic Unit (ALU)
- Control flow and branching
- Execution tracing and debugging output

### Built-in Assembler

Translates assembly source code into executable machine instructions without any external toolchain. Supports:

- Label declaration and resolution
- Operand parsing and validation
- Syntax error reporting
- Binary machine instruction generation

### Modular Emulator Architecture

| Module | Responsibility |
|--------|---------------|
| `registers` | CPU register file management |
| `memory` | Simulated RAM read/write |
| `alu` | Arithmetic and logical operations |
| `decoder` | Instruction fetch and decode |
| `control` | Program counter and execution flow |

### WebAssembly Deployment

Compiled via **Emscripten**, the emulator runs identically as a native binary, inside Node.js, or directly in a browser — no changes to the C source required.

### React Frontend

A browser-based IDE where users can write assembly programs, execute them in the emulator, and inspect output — all running locally via the WebAssembly module.

---

## Architecture Overview

```
Assembly Source (.txt)
        │
        ▼
  ┌─────────────┐
  │  Assembler  │  ← Lexing, parsing, label resolution
  └──────┬──────┘
         │  Machine Instructions
         ▼
  ┌─────────────────────────────────┐
  │            CPU Core             │
  │  ┌─────────┐   ┌─────────────┐  │
  │  │Registers│   │   Memory    │  │
  │  └─────────┘   └─────────────┘  │
  │  ┌─────────┐   ┌─────────────┐  │
  │  │   ALU   │   │   Decoder   │  │
  │  └─────────┘   └─────────────┘  │
  │         ┌──────────┐            │
  │         │ Control  │            │
  │         └──────────┘            │
  └─────────────────────────────────┘
         │
         ▼
  Execution Trace / Output
```

---

## Project Structure

```
Map2616/
│
├── src/                        # Emulator and assembler implementation
│   ├── main.c                  # Entry point
│   ├── assembler.c             # Assembler: lexer, parser, code gen
│   ├── cpu.c                   # CPU execution loop
│   ├── alu.c                   # Arithmetic and logic unit
│   ├── decoder.c               # Instruction decode logic
│   ├── registers.c             # Register file
│   ├── memory.c                # Memory simulation
│   └── control.c               # Program counter and control flow
│
├── include/                    # Header files
│   ├── assembler.h
│   ├── cpu.h
│   ├── alu.h
│   ├── decoder.h
│   ├── registers.h
│   ├── memory.h
│   └── control.h
│
├── frontend/                   # React frontend (browser interface)
│   ├── public/
│   ├── src/
│   │   ├── App.jsx             # Main app component
│   │   └── components/         # UI components (editor, output, controls)
│   └── package.json
│
├── examples/                   # Sample assembly programs
│   └── hello.txt
│
├── build/                      # Native binary build output (git-ignored)
├── build-wasm/                 # WebAssembly build output (git-ignored)
│
├── CMakeLists.txt              # CMake build configuration
├── .gitignore
└── README.md
```

---

## Getting Started

### Prerequisites

**Linux:**
- GCC or Clang
- CMake ≥ 3.10
- (Optional) Emscripten SDK — for WebAssembly builds
- (Optional) Node.js ≥ 16 — for the React frontend

**Windows:**
- WSL2 (Ubuntu recommended) — required for native and WASM builds
- Visual Studio Build Tools or MinGW — for native Windows builds
- CMake (via `winget install Kitware.CMake` or the installer at cmake.org)
- Node.js — for the React frontend

---

### Native Build (Linux)

```bash
git clone https://github.com/IwillCrushYou/Map2616
cd Map2616
mkdir build && cd build
cmake ..
make
./main /path/to/program.txt
```

To enable verbose execution tracing, pass a debug flag if supported:

```bash
./main /path/to/program.txt --trace
```

---

### WebAssembly Build

Requires the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html).

```bash
# Activate Emscripten environment
source /path/to/emsdk/emsdk_env.sh

mkdir build-wasm && cd build-wasm
emcmake cmake ..
emmake make
```

To run in Node.js:

```bash
node main.js
```

---

### React Frontend

```bash
cd frontend
npm install
npm start
```

Open `http://localhost:3000` in your browser. The frontend loads the WebAssembly module from `build-wasm/` and exposes a full assembly editor with live output.

> **Note:** Build the WebAssembly target before starting the frontend, as the React app depends on the compiled `.wasm` file.

---

## Assembly Language Reference

Below is an overview of the Map2616 assembly syntax. See `examples/` for working programs.

```asm
; Register names: R0–R7
; Immediate values: decimal or 0x hex prefix
; Labels: followed by colon (loop:)

    MOV R0, 5        ; Load immediate into R0
    MOV R1, 0x0A     ; Load hex immediate into R1
    ADD R0, R1       ; R0 = R0 + R1
    SUB R2, R0, R1   ; R2 = R0 - R1
    JMP loop         ; Unconditional jump to label
loop:
    DEC R0           ; Decrement R0
    JNZ loop         ; Jump if R0 != 0
    HLT              ; Halt execution
```

Full instruction set documentation is planned — contributions welcome.

---

## Contributing

Contributions are welcome! Please open an issue before submitting large changes so the approach can be agreed upon first.

---

### Contributing on Linux

**1. Fork and clone the repository**

```bash
git clone https://github.com/YOUR_USERNAME/Map2616
cd Map2616
```

**2. Create a feature branch**

```bash
git checkout -b feature/your-feature-name
```

**3. Make your changes**

Follow the existing code style. New modules should have a corresponding header in `include/`. Keep the modular architecture intact — each component (ALU, decoder, memory, etc.) should remain self-contained.

**4. Build and test**

```bash
mkdir -p build && cd build
cmake ..
make
./main ../examples/hello.txt
```

Verify your changes don't break existing example programs.

**5. Commit and push**

```bash
git add .
git commit -m "feat: describe your change clearly"
git push origin feature/your-feature-name
```

**6. Open a pull request** on GitHub against the `main` branch.

---

### Contributing on Windows

Windows development is supported via **WSL2** (recommended) or **MinGW**.

#### Option A — WSL2 (Recommended)

WSL2 gives you a full Linux environment inside Windows, which is the easiest path to a working build.

**1. Install WSL2**

Open PowerShell as Administrator and run:

```powershell
wsl --install
```

Restart when prompted. Ubuntu will be installed by default. Launch it from the Start Menu.

**2. Inside WSL2, follow the Linux instructions above exactly.**

Your Windows filesystem is accessible at `/mnt/c/`. You can clone into your Windows home directory and edit files with VS Code using the WSL extension.

**3. VS Code integration (recommended)**

Install the [WSL extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-wsl) for VS Code. Open the project folder inside WSL:

```bash
code .
```

This gives you full IntelliSense, debugging, and terminal access from within WSL.

---

#### Option B — Native Windows with MinGW + CMake

If you prefer not to use WSL2, you can build natively on Windows.

**1. Install the required tools**

```powershell
winget install Kitware.CMake
winget install MSYS2.MSYS2
```

Inside the MSYS2 terminal, install MinGW:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake make
```

Add `C:\msys64\mingw64\bin` to your system `PATH`.

**2. Clone the repository**

```powershell
git clone https://github.com/YOUR_USERNAME/Map2616
cd Map2616
```

**3. Build with CMake**

```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
.\main.exe ..\examples\hello.txt
```

**4. Notes for Windows contributors**

- Line endings: configure Git to use Unix-style endings to avoid diff noise.

  ```powershell
  git config --global core.autocrlf input
  ```

- WebAssembly builds on Windows require Emscripten inside WSL2. Native Windows Emscripten support is experimental and not recommended.

- React frontend development works natively on Windows — just install Node.js from [nodejs.org](https://nodejs.org) and run `npm install && npm start` inside the `frontend/` directory.

---

## Commit Message Convention

Use the following prefixes for clarity:

| Prefix | Use for |
|--------|---------|
| `feat:` | New feature or instruction |
| `fix:` | Bug fix |
| `docs:` | Documentation changes |
| `refactor:` | Code restructuring without behavior change |
| `test:` | Adding or updating tests |
| `build:` | CMake or build system changes |

---

## License

This project is open source. See [LICENSE](LICENSE) for details.
