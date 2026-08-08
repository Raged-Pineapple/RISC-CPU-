# RISC-V CPU Simulator

A 64-bit RISC-V CPU emulator written in C++, implementing the classic 5-stage pipeline with branch prediction, cache hierarchy modeling, and full instruction-level tracing.

Built from scratch as part of a Computer Architecture course project. The goal is to make the internals of a real pipelined processor visible and measurable — you can watch every instruction move through each stage, see hazards being detected and resolved, compare branch prediction strategies, and run your own C programs compiled for RISC-V.

---

## What it does

The emulator loads a RISC-V ELF binary, maps it into a virtual memory space, and runs it through a simulated 5-stage pipeline:

```
Fetch  →  Decode  →  Execute  →  Memory Access  →  Write Back
```

At the end of every run, it reports:

- Total instructions executed
- Total clock cycles and average CPI (cycles per instruction)
- Number of data hazards, control hazards, and memory hazards
- Branch prediction accuracy and strategy used

It also supports verbose mode (prints pipeline state every cycle), single-step mode (pauses after each instruction), and a full memory and register dump.

---

## Features

- **54 RV64I instructions** — covers the full base integer instruction set including arithmetic, logical, shift, load/store, branches, jumps, and system calls
- **5-stage pipeline** — faithful to the Patterson & Hennessy textbook model, with forwarding paths and stall detection
- **Hazard detection** — data hazards, control hazards, and memory (load-use) hazards are all detected, counted, and handled
- **4 branch prediction strategies** — Always Taken, Always Not Taken, Back-Taken Forward-Not-Taken, and a 2-bit Branch Prediction Buffer with saturating counters
- **3-level cache hierarchy** — L1 (32 KB), L2 (256 KB), L3 (8 MB), each configurable in size, associativity, block size, and hit/miss latency
- **ELF loader** — reads standard ELF64 binaries directly using the ELFIO library
- **Instruction-level tracing** — verbose mode prints full pipeline register state at every cycle
- **Memory and register dump** — saves complete execution history to `dump.txt` for post-run analysis

---

## Prerequisites

- Linux or WSL2 (Ubuntu recommended)
- `g++` with C++11 support
- `cmake` >= 3.1
- `make`

On Ubuntu or WSL:

```bash
sudo apt update
sudo apt install git build-essential cmake
```

---

## Build

```bash
git clone https://github.com/Raged-Pineapple/RISC-CPU-.git
cd RISC-CPU-
mkdir build
cd build
cmake ..
make
```

Three executables are produced in `build/`:

| Executable | What it does |
|---|---|
| `Simulator` | Runs a RISC-V ELF binary through the pipelined CPU emulator |
| `CacheSim` | Standalone cache behavior analyzer using memory trace files |
| `CacheOptimized` | Runs cache-optimized workloads for comparison experiments |

---

## Running the built-in programs

The `riscv-elf/` folder contains several pre-built RISC-V programs ready to run:

```bash
cd RISC-CPU-

./build/Simulator riscv-elf/helloworld.riscv
./build/Simulator riscv-elf/quicksort.riscv
./build/Simulator riscv-elf/matrixmulti.riscv
./build/Simulator riscv-elf/ackermann.riscv
./build/Simulator riscv-elf/test_arithmetic.riscv
./build/Simulator riscv-elf/test_branch.riscv
```

Each run prints the program output followed by a statistics block:

```
Hello, World!
Program exit from an exit() system call
------------ STATISTICS -----------
Number of Instructions: 152
Number of Cycles: 252
Avg Cycles per Instruction: 1.6579
Branch Prediction Accuracy: 0.5294 (Strategy: Always Not Taken)
Number of Control Hazards: 25
Number of Data Hazards: 73
Number of Memory Hazards: 1
-----------------------------------
```

---

## Command-line options

```
./build/Simulator <elf-file> [-v] [-s] [-d] [-b <strategy>]
```

| Flag | Description |
|---|---|
| `-v` | Verbose — prints the full pipeline register state at every cycle |
| `-s` | Single-step — pauses execution after each instruction (use with `-v`) |
| `-d` | Dump — writes full instruction and register history to `dump.txt` |
| `-b <strategy>` | Sets the branch prediction strategy (default: `NT`) |

### Branch prediction strategies

| Value | Strategy | How it works |
|---|---|---|
| `NT` | Always Not Taken | Predicts no branch is ever taken |
| `AT` | Always Taken | Predicts every branch is always taken |
| `BTFNT` | Back-Taken, Forward Not-Taken | Predicts backward branches (loops) as taken, forward branches as not taken |
| `BPB` | Branch Prediction Buffer | Uses a 4096-entry table of 2-bit saturating counters, updated after each branch resolves |

### Examples

```bash
# Run with verbose output saved to a file
./build/Simulator riscv-elf/quicksort.riscv -v > trace.txt

# Single-step through a program (press Enter to advance)
./build/Simulator riscv-elf/helloworld.riscv -v -s

# Dump memory and register history
./build/Simulator riscv-elf/quicksort.riscv -d
cat dump.txt

# Compare branch prediction strategies
./build/Simulator riscv-elf/ackermann.riscv -b AT
./build/Simulator riscv-elf/ackermann.riscv -b NT
./build/Simulator riscv-elf/ackermann.riscv -b BTFNT
./build/Simulator riscv-elf/ackermann.riscv -b BPB
```

---

## Writing and running your own C programs

You can write a C program, cross-compile it to RISC-V, and run it through the emulator. Here is the full process.

### Step 1 — Install the RISC-V cross-compiler

```bash
sudo apt install gcc-riscv64-unknown-elf
```

Verify:

```bash
riscv64-unknown-elf-gcc --version
```

### Step 2 — Write your C program

The emulator does not link the standard C library. Instead, use the I/O functions in `test/lib.h`:

```c
void print_s(const char *str);   // print a string
void print_d(int num);           // print a decimal integer
void print_c(char ch);           // print a single character
char read_char();                 // read one character from stdin
long long read_num();             // read a number from stdin
void exit_proc();                 // exit (must be called at end of main)
```

Example — Fibonacci:

```c
#include "lib.h"

int fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

int main() {
    print_s("Fibonacci sequence:\n");
    int i;
    for (i = 0; i < 15; i++) {
        print_s("fib(");
        print_d(i);
        print_s(") = ");
        print_d(fib(i));
        print_s("\n");
    }
    exit_proc();
}
```

Save this as `test/myprog.c`.

### Step 3 — Cross-compile

```bash
riscv64-unknown-elf-gcc \
  -march=rv64i -mabi=lp64 \
  -nostdlib -nostartfiles \
  -Wl,--entry=main \
  test/myprog.c test/lib.c \
  -o riscv-elf/myprog.riscv
```

Flag explanations:

| Flag | Reason |
|---|---|
| `-march=rv64i` | Target only the base RV64I instruction set |
| `-mabi=lp64` | 64-bit ABI, no hardware floating point |
| `-nostdlib -nostartfiles` | No standard C library (bare-metal) |
| `-Wl,--entry=main` | Use `main` as the program entry point |
| `test/lib.c` | Always include alongside your source |

### Step 4 — Run it

```bash
./build/Simulator riscv-elf/myprog.riscv
```

### Step 5 — Experiment with branch prediction

```bash
./build/Simulator riscv-elf/myprog.riscv -b AT
./build/Simulator riscv-elf/myprog.riscv -b NT
./build/Simulator riscv-elf/myprog.riscv -b BTFNT
./build/Simulator riscv-elf/myprog.riscv -b BPB
```

Compare the cycle counts to see which prediction strategy works best for your specific program's branching patterns.

---

## Cache hierarchy

The emulator models a 3-level cache that sits between the CPU and main memory. The default configuration mirrors a typical modern processor:

| Level | Size | Block size | Associativity | Hit latency | Miss latency |
|---|---|---|---|---|---|
| L1 | 32 KB | 64 B | 8-way | 0 cycles | 8 cycles |
| L2 | 256 KB | 64 B | 8-way | 8 cycles | 20 cycles |
| L3 | 8 MB | 64 B | 8-way | 20 cycles | 100 cycles |

Cache parameters can be modified directly in `src/MainCPU.cpp`. The `CacheSim` tool runs standalone cache experiments using memory trace files from the `cache-trace/` folder.

---

## Project structure

```
RISC-CPU-/
├── src/
│   ├── Simulator.cpp / Simulator.h         — pipeline core: fetch, decode, execute,
│   │                                          memory access, write back, hazard handling
│   ├── BranchPredictor.cpp / BranchPredictor.h  — all 4 prediction strategies
│   ├── Cache.cpp / Cache.h                 — cache model with LRU eviction
│   ├── MemoryManager.cpp / MemoryManager.h — virtual memory and page management
│   ├── MainCPU.cpp                         — entry point for Simulator, ELF loader
│   ├── MainCache.cpp                       — entry point for CacheSim
│   └── MainCacheOptimization.cpp           — entry point for CacheOptimized
├── include/
│   └── elfio/                              — ELFIO library for parsing ELF binaries
├── test/
│   ├── lib.h / lib.c                       — bare-metal I/O library for custom programs
│   ├── helloworld.c
│   ├── quicksort.c
│   ├── matrixmulti.c
│   ├── ackermann.c
│   ├── fib.c                               — Fibonacci example (custom)
│   └── test_arithmetic.c / test_branch.c / test_syscall.c
├── riscv-elf/                              — pre-built ELF binaries and disassembly dumps
├── cache-trace/                            — memory trace files for CacheSim
└── CMakeLists.txt
```

---

## How the pipeline works

Each instruction goes through 5 stages per clock cycle:

1. **Fetch** — reads the 32-bit instruction at the current PC from memory
2. **Decode** — identifies the instruction type, reads source registers, computes operands and branch target
3. **Execute** — the ALU performs the operation; branch prediction is resolved here
4. **Memory Access** — load and store instructions access the cache hierarchy
5. **Write Back** — results are written back to the destination register

When a hazard is detected:
- **Data hazard** — a stall bubble is inserted and the pipeline waits for the value to be available (or forwarded from a later stage)
- **Control hazard** — if branch prediction is wrong, the incorrectly fetched instructions are flushed and the correct PC is restored
- **Memory hazard** — a load-use hazard causes a one-cycle stall before the loaded value is consumed

All hazard counts are tracked separately and reported in the statistics output.

---

## Supported instructions

The emulator supports 54 instructions across all RV64I formats:

- **Arithmetic**: `ADD`, `SUB`, `ADDI`, `ADDIW`, `ADDW`, `SUBW`, `MUL`, `MULH`, `DIV`, `REM`
- **Logical**: `AND`, `OR`, `XOR`, `ANDI`, `ORI`, `XORI`
- **Shift**: `SLL`, `SRL`, `SRA`, `SLLI`, `SRLI`, `SRAI`, `SLLIW`, `SRLIW`, `SRAIW`, `SLLW`, `SRLW`, `SRAW`
- **Compare**: `SLT`, `SLTU`, `SLTI`, `SLTIU`
- **Load**: `LB`, `LH`, `LW`, `LD`, `LBU`, `LHU`, `LWU`
- **Store**: `SB`, `SH`, `SW`, `SD`
- **Branch**: `BEQ`, `BNE`, `BLT`, `BGE`, `BLTU`, `BGEU`
- **Jump**: `JAL`, `JALR`
- **Upper immediate**: `LUI`, `AUIPC`
- **System**: `ECALL`

---

## Original project

This repository is based on the open-source [RISCV-Simulator](https://github.com/hehao98/RISCV-Simulator) by He Hao, originally written for PKU Computer Architecture Labs (Spring 2019).
