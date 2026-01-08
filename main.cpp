#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "utils.h"
using namespace std;

int main(){
    cout << "Enter the program file name to run:" << endl;
    string fileName;
    getline(cin, fileName);
    ifstream myfile(fileName);

    if (!myfile.is_open()){
        cout << "Invalid." << endl;
        return 1;
    }

    // record all instructions in program
    vector<string> program;
    string instruction;
    while (getline(myfile, instruction)){
        program.push_back(instruction);
    }
    myfile.close();

    // initialize appropriate values
    if (fileName == "sample_part1.txt"){
        initValuesPartOne();
    } else if (fileName == "sample_part2.txt"){
        initValuesPartTwo();
    }

    // run the code while we are still within the program
    while (pc/4 < program.size()){

        string inst = Fetch(program);
        Decode(inst);

        int alu_result = Execute();

        // Mem only does something if MemRead or MemWrite
        int mem_result = 0;
        if (MemRead || MemWrite){
            mem_result = Mem();
        } 

        // depends if its a mem instruction
        if (MemRead) {
            Writeback(mem_result);
        }
        else {
            Writeback(alu_result);
        }

        pcUpdate();

        // print out necessary info from assignment pdf
        cout << endl << "total_clock_cycles " << total_clock_cycles << " :" << endl;
        if (RegWrite) {
            cout << reg_name[rd] << " is modified to 0x" << hex << rf[rd] << dec << endl;
        }
        if (MemWrite) {
            cout << "memory 0x" << hex << mem_result << " is modified to 0x" << d_mem[mem_result/4] << dec << endl;
        }
        // always
        cout << "pc is modified to 0x" << hex << pc << dec << endl;
    
    }

    cout << endl << "program terminated:" << endl;
    
    return 0;
}