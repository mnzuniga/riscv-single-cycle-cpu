#ifndef UTILS_H
#define UTILS_H

// global vars
int pc = 0;
int next_pc = 0;
int branch_target = 0;
int alu_zero = 0;
int total_clock_cycles = 0;

// control signals
bool RegWrite = false;
bool MemtoReg = false;
bool MemRead = false;
bool MemWrite = false;
bool Branch = false;
bool ALUSrc = false; // use immediate as 2nd alu operand
int ALUOp = 0;
bool Jump = false; // for jal
bool JumpReg = false; // for jalr
int alu_ctrl = 0;

// registers and memory
int rf[32] = {0};
int d_mem[32] = {0};

// register names 
std::string reg_name[32] = {
    "zero",
    "ra",
    "sp",
    "gp",
    "tp",
    "t0", "t1", "t2",
    "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"
};

void initValues(){
    rf[1] = 0x20;
    rf[2] = 0x5;
    rf[10] = 0x70;
    rf[11] = 0x4;
    d_mem[28] = 0x5; // 28 = 0x70 / 4
    d_mem[29] = 0x10; // 29 = 0x74 / 4
}
void initValuesPartOne(){
    rf[1] = 0x20;
    rf[2] = 0x5;
    rf[10] = 0x70;
    rf[11] = 0x4;
    d_mem[28] = 0x5; // 28 = 0x70 / 4
    d_mem[29] = 0x10; // 29 = 0x74 / 4
}
void initValuesPartTwo(){
    rf[8]  = 0x20; 
    rf[10] = 0x5; 
    rf[11] = 0x2;
    rf[12] = 0xa;
    rf[13] = 0xf;
}
    
std::string Fetch(std::vector<std::string>& program){
    // fetch one instruction per cycle. the txt file will have one inst per line
    int index = pc/4; // which instruction am i doing based on PC
    if (index >= program.size()){
        return ""; // over
    }
    std::string inst = program[index];
    next_pc = pc + 4;

    return inst; // return the instruction to be decoded

}

// global vars for decode
std::string op_code;
int rs1, rs2, rd;
int immediate = 0;

int ControlUnit(std::string &opcode){
    RegWrite = MemtoReg = MemRead = MemWrite = Branch = Jump = JumpReg = ALUSrc = false;
    ALUOp  = 0;

    // separate by types and opcodes
    if (opcode == "0110011") { // r
        RegWrite = true;
        ALUSrc   = false;
        ALUOp    = 2;
    }
    else if (opcode == "0010011") { // i
        RegWrite = true;
        ALUSrc   = true;
        ALUOp    = 0;
    }
    else if (opcode == "0000011") { // lw
        RegWrite = true;
        MemtoReg = true;
        MemRead  = true;
        ALUSrc   = true;
        ALUOp    = 0;
    }
    else if (opcode == "0100011") { // sw
        MemWrite = true;
        ALUSrc   = true;
        ALUOp    = 0;
    }
    else if (opcode == "1100011") { // beq
        Branch  = true;
        ALUOp   = 1;
    }
    else if (opcode == "1101111") { // jal
        RegWrite = true;
        Jump     = true;
    }
    else if (opcode == "1100111") { // jalr
        RegWrite  = true;
        JumpReg   = true;
        ALUSrc    = true;
    }
    return 0;
}

void ALUControl(const std::string &funct3, const std::string &funct7){
    // changes alu_ctrl
    if (ALUOp == 2){
        if (funct7 == "0000000" && funct3 == "000"){
            alu_ctrl = 0b0010; // add
        } else if(funct7 == "0100000" && funct3 == "000"){
            alu_ctrl = 0b0110; // sub
        } else if(funct3 == "111"){
            alu_ctrl = 0b0000; // AND
        } else if(funct3 == "110"){
            alu_ctrl = 0b0001; // OR
        }
    } else if (ALUOp == 1){
        alu_ctrl = 0b0110; // subtract for branching
    } else{
        if (funct3 == "111"){
            alu_ctrl = 0b0000; // andi
        } else if (funct3 == "110"){
            alu_ctrl = 0b0001; //ori
        } else{
            alu_ctrl = 0b0010; // add .. default 
        }
    }
}

int Decode(std::string &inst){
    std::string opcode_field = inst.substr(25, 7); // opcode is the first 7 bits in little endian

    // generate all control signals
    ControlUnit(opcode_field);

    if (opcode_field == "0110011") {  // r type
        rd = std::stoi(inst.substr(20, 5), nullptr, 2);
        std::string funct3 = inst.substr(17, 3);
        rs1 = std::stoi(inst.substr(12, 5), nullptr, 2);
        rs2 = std::stoi(inst.substr(7, 5), nullptr, 2);
        std::string funct7 = inst.substr(0, 7);
        // get alu control code:
        ALUControl(funct3, funct7);
    }
    else if (opcode_field == "0010011") {  // i type
        rd  = std::stoi(inst.substr(20, 5), nullptr, 2);
        std::string funct3 = inst.substr(17, 3);
        rs1 = std::stoi(inst.substr(12, 5), nullptr, 2);
        // immediate is [0:11] in little endian
        immediate = std::stoi(inst.substr(0, 12), nullptr, 2);
        // sign-extension from google gemini
        if (immediate & (1 << 11)){
            immediate = immediate - (1 << 12);
        }
        ALUControl(funct3, "");
    }
    else if (opcode_field == "0000011") {  // lw
        rd  = std::stoi(inst.substr(20, 5), nullptr, 2);
        std::string funct3 = inst.substr(17, 3);
        rs1 = std::stoi(inst.substr(12, 5), nullptr, 2);
        immediate = std::stoi(inst.substr(0, 12), nullptr, 2);
        if (immediate & (1 << 11)){
            immediate = immediate - (1 << 12);
        }
        ALUControl(funct3, "");
    }
    else if (opcode_field == "0100011") {  // sw
        std::string imm_high = inst.substr(0, 7);
        std::string imm_low  = inst.substr(20, 5);
        // put them together
        immediate = std::stoi(imm_high + imm_low, nullptr, 2);
        if (immediate & (1 << 11)){
            immediate -= (1 << 12);
        }

        rs1 = std::stoi(inst.substr(12, 5), nullptr, 2);
        rs2 = std::stoi(inst.substr(7, 5), nullptr, 2);
        std::string funct3 = inst.substr(17, 3);
        ALUControl(funct3, "");
    }
    else if (opcode_field == "1100011") {  // beq
        std::string imm_bit12  = inst.substr(0,1);
        std::string imm_bits10_5 = inst.substr(1, 6);
        std::string imm_bits4_1 = inst.substr(20, 4);
        std::string imm_bit11  = inst.substr(24, 1);
        std::string imm_str = imm_bit12 + imm_bit11 + imm_bits10_5 + imm_bits4_1 + "0"; // last bit is always 0
        immediate = std::stoi(imm_str, nullptr, 2);
        if (immediate & (1 << 12)){
            immediate = immediate - (1 << 13);
        }

        rs1 = std::stoi(inst.substr(12, 5), nullptr, 2);
        rs2 = std::stoi(inst.substr(7, 5), nullptr, 2);

        std::string funct3 = inst.substr(17, 3);
        ALUControl(funct3, "");
    } 
    else if (opcode_field == "1101111"){ // jal
        rd = std::stoi(inst.substr(20,5), nullptr, 2);
        std::string imm20 = inst.substr(0,1);
        std::string imm10_1 = inst.substr(1,10);
        std::string imm11 = inst.substr(11,1);
        std::string imm19_12 = inst.substr(12,8);
        std::string imm_str = imm20 + imm19_12 + imm11 + imm10_1 + "0";
        immediate = std::stoi(imm_str, nullptr, 2);
        if (immediate & (1 << 20)){
            immediate = immediate - (1 << 21);
        }
    }
    else if (opcode_field == "1100111"){ // jalr
        rd = std::stoi(inst.substr(20,5), nullptr, 2);
        rs1 = std::stoi(inst.substr(12,5), nullptr, 2);
        immediate = std::stoi(inst.substr(0,12), nullptr, 2);
        if (immediate & (1 << 11)){
            immediate = immediate - (1 << 12);
        }
    }

    return 0;
}

int Execute(){
    // first do jal and jalr
    if (Jump && !JumpReg){
        branch_target = pc + immediate;
        alu_zero = 0;
        return 0;
    } else if (JumpReg){
        // jalr
        branch_target = (rf[rs1] + immediate) & ~1; // clear the last bit using bitwise and. from google gemini
        // this is the address we will jump to
        alu_zero = 0;
        return 0;
    }

    int alu_result = 0;
    int op1 = rf[rs1];
    int op2;
    if (ALUSrc){
        op2 = immediate;
    } else{
        op2 = rf[rs2];
    }
    // do operation based on alu ctrl
    switch(alu_ctrl){
        case 0b0000:
            alu_result = op1 & op2;
            break;
        
        case 0b0001:
            alu_result = op1 | op2;
            break;
        
        case 0b0010:
            alu_result = op1 + op2;
            break;

        case 0b0110:
            alu_result = op1 - op2;
            break;
    }

    if (alu_result == 0) {
        alu_zero = 1;
    } else {
        alu_zero = 0;
    }
    
    // branch target calculation
    if (Branch){
        branch_target = next_pc + (immediate << 1);
    }

    return alu_result;
}

// for lw and sw
int Mem(){
    int given_address = rf[rs1] + immediate; // address we are working with from rs1
    int mem_index = given_address/4; // what index it matches to
    
    if (MemRead) {
        // read memory content. for lw
        return d_mem[mem_index];
    } else if (MemWrite) {
        // write rs2 to memory. for sw
        d_mem[mem_index] = rf[rs2];
        return given_address;
    }
    return 0;
}

int Writeback(int result){
    if (RegWrite){
        if (Jump || JumpReg){
            // jal and jalr
            rf[rd] = next_pc;
        } else{
            rf[rd] = result;
        }
    }
    total_clock_cycles += 1; // increment clock cycles
    return 0;
}

void pcUpdate(){
    if (Jump && !JumpReg){
        pc = branch_target;
    } else if(JumpReg){
        pc = branch_target;
    } 
    else if(Branch && alu_zero){
        pc = branch_target;
    } 
    else{
        pc = next_pc;
    }
}


#endif