#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define REGISTER unsigned int

#define MEMORY int

#define REG_NUM 32
#define MEM_SIZE 256 //  1024/4

#define INSTRUCTION_LENGTH 32
#define INITIAL_COM_ADDR 256
#define FLAG_COM_ADDR -1

#define BREAK_CODE "01010100000000000000000000001101"

using namespace std;

/*
cstr compare class
*/

class cstr_compare {
public:
	bool operator()(const char *l, const char * r) {
		if (strcmp(l, r) < 0)
			return true;
		else
			return false;
	}
};

/*
console class
*/

class Console{
private:
    int reg[REG_NUM];
    int mem[MEM_SIZE];
    MEMORY start_mem;
    MEMORY end_mem;
    int cycle_count;
    FILE *out;
    Console(){};
public:
    int PC;
    Console(int m_pc, int m_start_mem, int m_end_start,const char* fname);
    ~Console();
    
    void execute(const char* command);
    
    inline void memory_set(MEMORY addr, int value)
    {
        mem[addr] = value;
    }
    
    
    
    void internal_show()
    {
        printf("REG SHOW\n");
        for (int i=0; i<REG_NUM; i++) {
            printf("R[%d]=%d ",i, reg[i]);
        }
        printf("\n");
    }
    
    void simu_print(const char *command);
    
    
};

/*
Disassembler class
*/
#pragma once
class Disassembler{
private:
	map<char*, char*, cstr_compare> category1;
	map<char*, char*, cstr_compare> category2;
    
public:
//    vector<char*> result;
    map<MEMORY, char*> result;
    
public:
	Disassembler();
    ~Disassembler();
    
	void disassemble(int type, int address,const char* line,char* out);
    
    // UnSignedImmediateTransform -> strtol
    
	int SignedImmediateTransform(const char* number,int length);
    
};
