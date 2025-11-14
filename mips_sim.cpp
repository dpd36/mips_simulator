#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;

// ---------------------------------------------------------------------
//  Configuration
// ---------------------------------------------------------------------
constexpr int REG_COUNT = 32;
constexpr int MEM_SIZE  = 1024;

// Register name to number mapping
unordered_map<string, int> regMap = {
    {"$zero", 0}, {"$t0", 8}, {"$t1", 9}, {"$t2", 10}, {"$t3", 11},
    {"$t4", 12}, {"$t5", 13}, {"$t6", 14}, {"$t7", 15},
    {"$s0", 16}, {"$s1", 17}, {"$s2", 18}, {"$s3", 19},
    {"$s4", 20}, {"$s5", 21}
};

int getRegNumber(const string& reg) {
    if (regMap.count(reg)) return regMap[reg];
    if (reg[0] == '$' && reg.size() > 1) return stoi(reg.substr(1));
    return 0;                     // $zero by default
}

// ---------------------------------------------------------------------
//  Instruction representation
// ---------------------------------------------------------------------
struct Instruction {
    string opcode;
    int rs = 0, rt = 0, rd = 0;
    int imm = 0;
    int address = -1;            // for BEQ / J
    string raw = "";
    bool valid = false;
    Instruction() = default;
};

// ---------------------------------------------------------------------
//  State registers
// ---------------------------------------------------------------------
struct IF_ID { Instruction inst; int npc = 0; };   // IF to ID
struct ID_EX { Instruction inst; int npc = 0;
               int rsVal = 0, rtVal = 0, imm = 0; };
struct EX_MEM { Instruction inst; int aluResult = 0, rtVal = 0; };
struct MEM_WB { Instruction inst; int memData = 0, aluResult = 0, rtVal = 0; };

// ---------------------------------------------------------------------
//  CPU state
// ---------------------------------------------------------------------
struct CPU {
    int regs[REG_COUNT] = {0};
    int mem[MEM_SIZE]   = {0};
    int pc = 0;
};

// ---------------------------------------------------------------------
//  Helper: strip comments
// ---------------------------------------------------------------------
string stripComments(const string& line) {
    size_t pos = line.find('#');
    return (pos == string::npos) ? line : line.substr(0, pos);
}

// ---------------------------------------------------------------------
//  Parse a line into an Instruction
// ---------------------------------------------------------------------
Instruction parseLine(const string& line,
                     const unordered_map<string, int>& labelMap) {
    Instruction inst;
    string s = line;
    while (!s.empty() && (s[0] == ' ' || s[0] == '\t')) s = s.substr(1);
    inst.raw = s;

    istringstream iss(s);
    iss >> inst.opcode;
    if (inst.opcode.empty()) { inst.valid = false; return inst; }
    inst.valid = true;

    if (inst.opcode == "ADDI") {
        string rt, rs, imm;
        iss >> rt >> rs >> imm;
        inst.rt = getRegNumber(rt.substr(0, rt.size() - 1));
        inst.rs = getRegNumber(rs.substr(0, rs.size() - 1));
        inst.imm = stoi(imm);
    }
    else if (inst.opcode == "ADD" || inst.opcode == "SUB" ||
             inst.opcode == "MUL" || inst.opcode == "AND" ||
             inst.opcode == "OR") {
        string rd, rs, rt;
        iss >> rd >> rs >> rt;
        inst.rd = getRegNumber(rd.substr(0, rd.size() - 1));
        inst.rs = getRegNumber(rs.substr(0, rs.size() - 1));
        inst.rt = getRegNumber(rt);
    }
    else if (inst.opcode == "SLL" || inst.opcode == "SRL") {
        string rd, rt, shamt;
        iss >> rd >> rt >> shamt;
        inst.rd = getRegNumber(rd.substr(0, rd.size() - 1));
        inst.rt = getRegNumber(rt.substr(0, rt.size() - 1));
        inst.imm = stoi(shamt);
    }
    else if (inst.opcode == "LW" || inst.opcode == "SW") {
        string rt, offset;
        iss >> rt >> offset;
        inst.rt = getRegNumber(rt.substr(0, rt.size() - 1));
        size_t lparen = offset.find('('), rparen = offset.find(')');
        inst.imm = stoi(offset.substr(0, lparen));
        inst.rs  = getRegNumber(offset.substr(lparen + 1,
                                            rparen - lparen - 1));
    }
    else if (inst.opcode == "BEQ") {
        string rs, rt, label;
        iss >> rs >> rt >> label;
        inst.rs = getRegNumber(rs.substr(0, rs.size() - 1));
        inst.rt = getRegNumber(rt.substr(0, rt.size() - 1));
        if (labelMap.count(label)) inst.address = labelMap.at(label);
    }
    else if (inst.opcode == "J") {
        string label;
        iss >> label;
        if (labelMap.count(label)) inst.address = labelMap.at(label);
    }
    else if (inst.opcode == "NOP") {
        inst.valid = false;
    }
    return inst;
}

// ---------------------------------------------------------------------
//  Print state
// ---------------------------------------------------------------------
void printCycleState(const CPU& cpu, int cycle, bool isDebug,
                     const IF_ID& ifid, const ID_EX& idex,
                     const EX_MEM& exmem, const MEM_WB& memwb) {
    if (isDebug) {
        cout << "\n=== CYCLE " << cycle << " ===\n\n";
    } else {
        cout << "\n=== FINAL STATE AFTER " << cycle << " CYCLES ===\n\n";
    }

    // Registers
    cout << "Registers:\n";
    cout << "----------\n";
    for (int i = 0; i < REG_COUNT; ++i) {
        cout << "$" << setw(2) << i << ":";
        cout << " " << setw(5) << left << cpu.regs[i];
        if ((i + 1) % 8 == 0) cout << "\n";
    }

    // Memory
    cout << "\nMemory (first 8 words):\n";
    cout << "-----------------------\n";
    for (int i = 0; i < 8; ++i) {
        cout << "[" << setw(2) << i*4 << "]:" << " "
             << setw(5) << left << cpu.mem[i];
    }
    cout << "\n";

    // Pipeline (debug only)
    if (isDebug) {
        cout << "\nPipeline Stages:\n";
        cout << "  IF/ID:  " << (ifid.inst.valid ? ifid.inst.raw : "(nop)") << "\n";
        cout << "  ID/EX:  " << (idex.inst.valid ? idex.inst.raw : "(nop)") << "\n";
        cout << "  EX/MEM: " << (exmem.inst.valid ? exmem.inst.raw : "(nop)") << "\n";
        cout << "  MEM/WB: " << (memwb.inst.valid ? memwb.inst.raw : "(nop)") << "\n";
    }
    cout << "\n";
}

// ---------------------------------------------------------------------
//  Main simulation
// ---------------------------------------------------------------------
int main(int argc, char* argv[]) {
    bool debug = (argc > 2 && string(argv[2]) == "--debug");
    string inFile = (argc > 1) ? argv[1] : "input.asm";

    // ---------- 1. Load program ----------
    vector<string> programLines;
    ifstream fin(inFile);
    string line;
    while (getline(fin, line)) {
        line = stripComments(line);
        if (line.empty()) continue;
        programLines.push_back(line);
    }

    // ---------- 2. First pass: label resolution ----------
    unordered_map<string, int> labelMap;
    vector<string> instLines;
    for (const auto& l0 : programLines) {
        string l = l0;
        size_t colon = l.find(':');
        if (colon != string::npos) {
            string label = l.substr(0, colon);
            size_t end = label.find_last_not_of(" \t");
            if (end != string::npos) label = label.substr(0, end + 1);
            if (!label.empty()) labelMap[label] = (int)instLines.size();
            l = l.substr(colon + 1);
        }
        size_t start = l.find_first_not_of(" \t");
        if (start != string::npos) l = l.substr(start);
        if (!l.empty()) instLines.push_back(l);
    }

    // ---------- 3. Build instruction memory ----------
    vector<Instruction> program;
    for (const auto& l : instLines)
        program.push_back(parseLine(l, labelMap));

    // ---------- 4. CPU & pipeline registers ----------
    CPU cpu;
    int cycle = 0;
    IF_ID   ifid;   // IF to ID
    ID_EX   idex;   // ID to EX
    EX_MEM  exmem;  // EX to MEM
    MEM_WB  memwb;  // MEM to WB

    int progSize = (int)program.size();
    bool branchTaken = false;
    int  branchTarget = -1;

    // temporary registers for the *next* cycle
    IF_ID   next_ifid{};
    ID_EX   next_idex{};
    EX_MEM  next_exmem{};
    MEM_WB  next_memwb{};

    // -----------------------------------------------------------------
    //  Main pipeline loop
    // -----------------------------------------------------------------
    while (cpu.pc < progSize ||
           ifid.inst.valid || idex.inst.valid ||
           exmem.inst.valid || memwb.inst.valid) {

        // -------------------- WB (Write-Back) --------------------
        if (memwb.inst.valid) {
            const string& op = memwb.inst.opcode;
            if (op == "ADD" || op == "SUB" || op == "MUL" ||
                op == "AND" || op == "OR" || op == "SLL" || op == "SRL") {
                cpu.regs[memwb.inst.rd] = memwb.aluResult;
            }
            else if (op == "ADDI") {
                cpu.regs[memwb.inst.rt] = memwb.aluResult;
            }
            else if (op == "LW") {
                cpu.regs[memwb.inst.rt] = memwb.memData;
            }
            cpu.regs[0] = 0;                 // $zero is hard-wired
        }

        // -------------------- MEM (Memory) --------------------
        next_memwb = {};
        next_memwb.inst      = exmem.inst;
        next_memwb.aluResult = exmem.aluResult;
        next_memwb.rtVal     = exmem.rtVal;

        if (exmem.inst.opcode == "LW") {
            int addr = exmem.aluResult / 4;
            next_memwb.memData = (addr >= 0 && addr < MEM_SIZE) ? cpu.mem[addr] : 0;
        }
        else if (exmem.inst.opcode == "SW") {
            int addr = exmem.aluResult / 4;
            if (addr >= 0 && addr < MEM_SIZE) cpu.mem[addr] = exmem.rtVal;
        }

        // -------------------- EX (Execute / ALU) --------------------
        next_exmem = {};
        next_exmem.inst  = idex.inst;
        next_exmem.rtVal = idex.rtVal;

        if (idex.inst.valid) {
            const string& op = idex.inst.opcode;
            if (op == "ADD")  next_exmem.aluResult = idex.rsVal + idex.rtVal;
            else if (op == "ADDI") next_exmem.aluResult = idex.rsVal + idex.imm;
            else if (op == "SUB")  next_exmem.aluResult = idex.rsVal - idex.rtVal;
            else if (op == "MUL")  next_exmem.aluResult = idex.rsVal * idex.rtVal;
            else if (op == "AND")  next_exmem.aluResult = idex.rsVal & idex.rtVal;
            else if (op == "OR")   next_exmem.aluResult = idex.rsVal | idex.rtVal;
            else if (op == "SLL")  next_exmem.aluResult = idex.rtVal << idex.imm;
            else if (op == "SRL")  next_exmem.aluResult = idex.rtVal >> idex.imm;
            else if (op == "LW" || op == "SW")
                next_exmem.aluResult = idex.rsVal + idex.imm;

            // Branch decision (taken in EX, flush in IF/ID)
            if (op == "BEQ" && idex.rsVal == idex.rtVal) {
                branchTaken  = true;
                branchTarget = idex.inst.address;
            }
            else if (op == "J") {
                branchTaken  = true;
                branchTarget = idex.inst.address;
            }
        }

        // -------------------- ID (Decode / Register Fetch) --------------------
        next_idex = {};
        next_idex.inst = ifid.inst;
        next_idex.npc  = ifid.npc;

        if (ifid.inst.valid) {
            const string& op = ifid.inst.opcode;
            if (op == "ADDI") {
                next_idex.rsVal = cpu.regs[ifid.inst.rs];
                next_idex.imm   = ifid.inst.imm;
            }
            else if (op == "ADD" || op == "SUB" || op == "MUL" ||
                     op == "AND" || op == "OR") {
                next_idex.rsVal = cpu.regs[ifid.inst.rs];
                next_idex.rtVal = cpu.regs[ifid.inst.rt];
            }
            else if (op == "SLL" || op == "SRL") {
                next_idex.rtVal = cpu.regs[ifid.inst.rt];
                next_idex.imm   = ifid.inst.imm;
            }
            else if (op == "LW" || op == "SW") {
                next_idex.rsVal = cpu.regs[ifid.inst.rs];
                next_idex.rtVal = cpu.regs[ifid.inst.rt];
                next_idex.imm   = ifid.inst.imm;
            }
            else if (op == "BEQ") {
                next_idex.rsVal = cpu.regs[ifid.inst.rs];
                next_idex.rtVal = cpu.regs[ifid.inst.rt];
            }
        }

        // -------------------- IF (Instruction Fetch) --------------------
        next_ifid = {};
        if (cpu.pc < progSize) {
            next_ifid.inst = program[cpu.pc];
            next_ifid.npc  = cpu.pc + 1;
            ++cpu.pc;
        }

        // -------------------- Branch / Jump Flush --------------------
        if (branchTaken) {
            cpu.pc = branchTarget;   // PC update for next cycle
            next_ifid = {};          // squash fetched instruction
            next_idex = {};          // squash decoded instruction
            branchTaken = false;
        }

        // -------------------- Debug print (per-cycle) --------------------
        if (debug) {
            printCycleState(cpu, cycle, true, ifid, idex, exmem, memwb);
        }

        ++cycle;

        // Advance pipeline registers
        memwb = next_memwb;
        exmem = next_exmem;
        idex  = next_idex;
        ifid  = next_ifid;
    }

    // ------------------------ Print final state -------------------------
    printCycleState(cpu, cycle - 1, false, ifid, idex, exmem, memwb);

    return 0;
}