# Dayton Deatherage, Sean Agyei, Jamie Jones, Ethan Sklar

# mips_sim

This program simulates a pipelined MIPS processor and displays the contents of the register
file and memory after execution. 

## Compilation

Compile the program with:
g++ -o mips_sim mips_sim.cpp

## Usage

Run the program with:
./mips_sim input_file

Run debug with:
./mips_sim input_file --debug

- `input_file`: Text file with mips assembly code

**Example:**

./mips_sim input
./mips_sim a.asm
