/*
Author: NIKUNJ SARDA
On my honor, I have neither given nor received unauthorized aid on this assignment.
*/

#include <iostream>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define REGISTER_SIZE 32
#define MEMORY_SIZE 256

#define INSTRUCTIONS_SIZE 32
#define INITIAL_COM_ADDR 256
#define FLAG_COM_ADDR -1

#define BREAK_CODE "01010100000000000000000000001101"

//for MIPSsim class
#define HYP "--------------------"

#define SIMULATION_OUTPUT_FORMAT(type,x) "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",\
                type[x],type[x+1],type[x+2],type[x+3], type[x+4],type[x+5],type[x+6],type[x+7]


//for main function

#define DISASSEMBLER_OUTPUT_FILE "disassembly.txt"
#define SIMULATION_OUTPUT_FILE "simulation.txt"

using namespace std;

/*
string compare class
*/

class stringCompare {
public:
	bool operator()(const char *l, const char * r) {
		if (strcmp(l, r) < 0)
			return true;
		else
			return false;
	}
};

/*
MIPSsim class
*/

class MIPSsim{

private:
    int reg[REGISTER_SIZE];
    int mem[MEMORY_SIZE];
    int start_mem;
    int end_mem;
    int cycle_count;
    FILE *out;
    map<char*, char*, stringCompare> category1;
	map<char*, char*, stringCompare> category2;

public:
    int PC;
    map<int, char*> result;
    MIPSsim(int m_pc, int m_start_mem, int m_end_start,const char* fname);
    MIPSsim();
    ~MIPSsim();
    
    void execute(const char* command);
    
    inline void memory_set(int addr, int value)
    {
        mem[addr] = value;
    }
    
    void simulationP(const char *command);

    void disassemble(int type, int address,const char* line,char* out);

    //as unsigned one's are handle by string to long conversion function
    int immediateTransform_Signed(const char* number,int length);
    
    
};

MIPSsim::MIPSsim(int m_pc, int m_start_mem, int m_end_mem, const char* fname)
{
    //class variable initialization
    PC = m_pc;
    start_mem = m_start_mem;
    end_mem = m_end_mem;
    cycle_count = 1;
    
    memset(reg, 0, sizeof(unsigned int)*REGISTER_SIZE);
    memset(mem, 0, sizeof(int)*MEMORY_SIZE);
    
    out = fopen(fname, "w");
}

//constructor to initialize instructions category
MIPSsim::MIPSsim(){
	category1["0000"] = "J";
	category1["0001"] = "JR";
	category1["0010"] = "BEQ";
	category1["0011"] = "BLTZ";
	category1["0100"] = "BGTZ";
	category1["0101"] = "BREAK";
	category1["0110"] = "SW";
	category1["0111"] = "LW";
	category1["1000"] = "SLL";
	category1["1001"] = "SRL";
	category1["1010"] = "SRA";
	category1["1011"] = "NOP";
    
	category2["0000"] = "ADD";
	category2["0001"] = "SUB";
	category2["0010"] = "MUL";
	category2["0011"] = "AND";
	category2["0100"] = "OR";
	category2["0101"] = "XOR";
	category2["0110"] = "NOR";
	category2["0111"] = "SLT";
	category2["1000"] = "ADDI";
	category2["1001"] = "ANDI";
	category2["1010"] = "ORI";
	category2["1011"] = "XORI";
}

//class destructor
MIPSsim::~MIPSsim(){
    map<int, char*>::iterator it;
    for(it = result.begin(); it != result.end(); it++)
    {
        free(it->second);
    }
    if (!out) {
        fclose(out);
    }
}

void MIPSsim::disassemble(int type, int address, const char* line, char* out){
	char opcode[5], map_opcode[10];
	unsigned int instr, sa;
    unsigned int rs, rt, rd, base;
    int offset, immediate;
    char* command = (char*)malloc(sizeof(char) * 30);
    
    memcpy(opcode, line + sizeof(char) * 2, sizeof(char) * 4);
    opcode[4] = '\0';
    
	if(type == 1){
        strcpy(map_opcode, category1[opcode]);
        
		if(strcmp(map_opcode, "J") == 0){
			char instr_index_str[27];
			memcpy(instr_index_str, line + sizeof(char) * 6, sizeof(char) * 27);
			instr = strtol(instr_index_str, NULL, 2) << 2;
            
			sprintf(out,"%s\t%d\t%s #%u",line, address, map_opcode, instr);
            sprintf(command, "%s #%u", map_opcode, instr);
            
            result[address] = command;
            
            return;
		}
        
        if (strcmp(map_opcode, "JR") == 0) {
            char rs_str[6];
            memcpy(rs_str, line + sizeof(char) * 6, sizeof(char) * 5);
            rs_str[5] = '\0';
            
            rs = strtol(rs_str, NULL, 2);
            
            sprintf(out, "%s\t%d\t%s R%u", line, address, map_opcode, rs);
            sprintf(command, "%s R%u", map_opcode, rs);
            
            result[address] = command;
            
            return;
        }
        
        if (strcmp(map_opcode, "BEQ") == 0) {
            char rs_str[6], rt_str[6], offset_str[17];
            memcpy(rs_str, line + sizeof(char) * 6, sizeof(char) * 5);
            rs_str[5] = '\0';
            memcpy(rt_str, line + sizeof(char) * 11, sizeof(char) * 5);
            rt_str[5] = '\0';
            memcpy(offset_str, line + sizeof(char) * 16, sizeof(char) * 17);
            
            rs = strtol(rs_str, NULL, 2);
            rt = strtol(rt_str, NULL, 2);
            offset = strtol(offset_str, NULL, 2) << 2;
            
            sprintf(out, "%s\t%d\t%s R%u, R%u, #%d",line,address,map_opcode,rs,rt,offset);
            sprintf(command, "%s R%u, R%u, #%d",map_opcode,rs,rt,offset);
            
            result[address] = command;
            
            return;
        }
        
        if (strcmp(map_opcode, "BLTZ") == 0 || strcmp(map_opcode, "BGTZ") == 0) {
            char rs_str[6], offset_str[17];
            memcpy(rs_str, line + sizeof(char) * 6, sizeof(char) * 5);
            rs_str[5] = '\0';
            memcpy(offset_str, line + sizeof(char) * 16, sizeof(char) * 17);
            
            rs = strtol(rs_str, NULL, 2);
            offset = strtol(offset_str, NULL, 2) << 2;
            
            sprintf(out, "%s\t%d\t%s R%u, #%d", line, address, map_opcode, rs, offset);
            sprintf(command, "%s R%u, #%d",map_opcode, rs, offset);
            
            result[address] = command;
            
            return;
        }
        
        if (strcmp(map_opcode, "BREAK") == 0 || strcmp(map_opcode, "NOP") == 0) {
            sprintf(out, "%s\t%d\t%s", line, address, map_opcode);
            sprintf(command, "%s", map_opcode);
            
            result[address] = command;
            
            return;
        }
        
        if (strcmp(map_opcode, "SW") == 0 || strcmp(map_opcode, "LW") == 0 ) {
            char base_str[6], rt_str[6], offset_str[17];
            memcpy(base_str, line + sizeof(char) * 6, sizeof(char) * 5);
            base_str[5] = '\0';
            memcpy(rt_str, line + sizeof(char) * 11, sizeof(char) * 5);
            rt_str[5] = '\0';
            memcpy(offset_str, line + sizeof(char) * 16, sizeof(char) * 17);
            
            base = strtol(base_str, NULL, 2);
            rt = strtol(rt_str, NULL, 2);
            offset = strtol(offset_str, NULL, 2);
            
            sprintf(out, "%s\t%d\t%s R%u, %d(R%u)", line, address, map_opcode, rt, offset, base);
            sprintf(command, "%s R%u, %d(R%u)", map_opcode, rt, offset, base);
            
            result[address] = command;
            
            return;
        }
        
        if (strcmp(map_opcode, "SLL") == 0 || strcmp(map_opcode, "SRL") == 0 
                || strcmp(map_opcode, "SRA") == 0) {
            char rt_str[6], rd_str[6], sa_str[6];
            memcpy(rt_str, line + sizeof(char) * 11, sizeof(char) * 5);
            rt_str[5] = '\0';
            memcpy(rd_str, line + sizeof(char) * 16, sizeof(char) * 5);
            rd_str[5] = '\0';
            memcpy(sa_str, line + sizeof(char) * 21, sizeof(char) * 5);
            sa_str[5] = '\0';
            
            rt = strtol(rt_str, NULL, 2);
            rd = strtol(rd_str, NULL, 2);
            sa = strtol(sa_str, NULL, 2);
            
            sprintf(out, "%s\t%d\t%s R%u, R%u, #%u", line, address, map_opcode, rd, rt, sa);
            sprintf(command, "%s R%u, R%u, #%u", map_opcode, rd, rt, sa);
            
            result[address] = command;
            
            return;
        }
        
        
	}
    
    //Category 2 operations
	else
	{
        strcpy(map_opcode, category2[opcode]);
        
		if(strcmp(map_opcode, "ADD") == 0 || strcmp(map_opcode, "SUB") == 0 ||
           strcmp(map_opcode, "MUL") == 0 || strcmp(map_opcode, "AND") == 0 ||
           strcmp(map_opcode, "OR") == 0 || strcmp(map_opcode, "XOR") == 0 ||
           strcmp(map_opcode, "NOR") == 0 || strcmp(map_opcode, "SLT") == 0){
			
			char rs_str[6], rt_str[6], rd_str[6];
			memcpy(rs_str, line + sizeof(char) * 6,sizeof(char) * 5);
            rs_str[5] = '\0';
			memcpy(rt_str, line + sizeof(char) * 11,sizeof(char) * 5);
            rt_str[5] = '\0';
			memcpy(rd_str, line + sizeof(char) * 16,sizeof(char) * 5);
            rd_str[5] = '\0';
            
			rs = strtol(rs_str, NULL, 2);
			rt = strtol(rt_str, NULL, 2);
			rd = strtol(rd_str, NULL, 2);
            
			sprintf(out, "%s\t%d\t%s R%u, R%u, R%u",line, address, map_opcode, rd, rs, rt);
            sprintf(command, "%s R%u, R%u, R%u", map_opcode, rd, rs, rt);
            
            result[address] = command;
            
            return;
		}
        
		if(strcmp(map_opcode, "ADDI") == 0 || strcmp(map_opcode, "ANDI") == 0||
           strcmp(map_opcode, "ORI") == 0 || strcmp(map_opcode, "XORI") == 0){
			char number_str[17];
			char rs_str[6], rt_str[6];
			memcpy(rs_str, line + sizeof(char) * 6, sizeof(char) * 5);
            rs_str[5] = '\0';
			memcpy(rt_str, line + sizeof(char) * 11, sizeof(char) * 5);
            rt_str[5] = '\0';
            
			rs = strtol(rs_str, NULL, 2);
			rt = strtol(rt_str, NULL, 2);
            
			memcpy(number_str, line + sizeof(char) * 16, sizeof(char) * 17);
			
			immediate = immediateTransform_Signed(number_str, 17-1);
            
			sprintf(out, "%s\t%d\t%s R%u, R%u, #%d", line, address, map_opcode, rt, rs, immediate);
            sprintf(command, "%s R%u, R%u, #%d", map_opcode, rt, rs, immediate);
            
            result[address] = command;
            
            return;
		}
	}
    
}

int MIPSsim::immediateTransform_Signed(const char* number, int length){
	if(number[0] == '0')
		return strtol(number, NULL, 2);
	else{
		char* left = (char*)malloc(sizeof(char) * length);
		memcpy(left, number + sizeof(char) * 1, sizeof(char) * length);
		int imm = strtol(left, NULL, 2) - (1<<31);
        
		free(left);
        
		return imm;
	} 
}

void MIPSsim::execute(const char* command)
{
    char oper[10];
    unsigned int rs, rt, rd, base;
    unsigned int instr, sa;
    int immediate, offset;
    sscanf(command, "%s", oper);
    
    if(strcmp(oper, "J") == 0)
    {
        sscanf(command, "%s #%u", oper, &instr);
        simulationP(command);
        
        PC = instr;
        
        goto end;
    }
    if(strcmp(oper, "JR") == 0)
    {
        sscanf(command, "%s R%u", oper, &rs);
        simulationP(command);
        
        PC = reg[rs];
        
        goto end;
    }
    if(strcmp(oper, "BEQ") == 0)
    {
        sscanf(command, "%s R%u, R%u, #%d", oper, &rs, &rt, &offset);
        simulationP(command);
        
        PC = (reg[rs] == reg[rt]) ? (PC + 4 + offset) : PC + 4;
        
        goto end;
    }
    if (strcmp(oper, "BLTZ") == 0) {
        sscanf(command, "%s R%u, #%d", oper, &rs, &offset);
        simulationP(command);
        
        PC = (reg[rs] < 0) ? (PC + 4 + offset): PC + 4;
        
        goto end;
    }
    if (strcmp(oper, "BGTZ") == 0) {
        sscanf(command, "%s R%u, #%d", oper, &rs, &offset);
        simulationP(command);
        
        PC = (reg[rs] > 0) ? (PC + 4 + offset) : PC + 4;
        
        goto end;
    }
    
    if (strcmp(oper, "BREAK") == 0) {
        simulationP(command);
        
        PC = FLAG_COM_ADDR;
        
        return;
    }
    
    if (strcmp(oper, "SW") == 0) {
        sscanf(command, "%s R%u, %d(R%u)", oper, &rt, &offset, &base);
        mem[(reg[base] + offset) / 4] = reg[rt];
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    if (strcmp(oper, "LW") == 0) {
        sscanf(command, "%s R%u, %d(R%u)", oper, &rt, &offset, &base);
        reg[rt] = mem[(reg[base] + offset) / 4];
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    if (strcmp(oper, "SLL") == 0) {
        sscanf(command, "%s R%u, R%u, #%u", oper, &rd, &rt, &sa);
        reg[rd] = reg[rt] << sa;
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    if (strcmp(oper, "SRL") == 0) {
        sscanf(command, "%s R%u, R%u, #%u", oper, &rd, &rt, &sa);
        reg[rd] = (unsigned)reg[rt] >>  sa;
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    if (strcmp(oper, "SRA") == 0) {
        sscanf(command, "%s R%u, R%u, #%u", oper, &rd, &rt, &sa);
        reg[rd] = reg[rt] >> sa;
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    if (strcmp(oper, "NOP") == 0) {
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    
    
    if (strcmp(oper, "ADD") == 0) {
        sscanf(command, "%s R%u, R%u, R%u", oper, &rd, &rs, &rt);
        reg[rd] = reg[rs] + reg[rt];
        simulationP(command);
        PC += 4;
        goto end;
    }
    if (strcmp(oper, "SUB") == 0) {
        sscanf(command, "%s R%u, R%u, R%u", oper, &rd, &rs, &rt);
        reg[rd] = reg[rs] - reg[rt];
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    if (strcmp(oper, "MUL") == 0) {
        sscanf(command, "%s R%u, R%u, R%u", oper, &rd, &rs, &rt);
        reg[rd] = reg[rs] * reg[rt];
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    if (strcmp(oper, "AND") == 0) {
        sscanf(command, "%s R%u, R%u, R%u", oper, &rd, &rs, &rt);
        reg[rd] = reg[rs] & reg[rt];
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }

    if (strcmp(oper, "OR") == 0) {
        sscanf(command, "%s R%u, R%u, R%u", oper, &rd, &rs, &rt);
        reg[rd] = reg[rs] | reg[rt];
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    if (strcmp(oper, "XOR") == 0) {
        sscanf(command, "%s R%u, R%u, R%u", oper, &rd, &rs, &rt);
        reg[rd] = reg[rs] ^ reg[rt];
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    if (strcmp(oper, "NOR") == 0) {
        sscanf(command, "%s R%u, R%u, R%u", oper, &rd, &rs, &rt);
        reg[rd] = ~(reg[rs] | reg[rt]);
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    if (strcmp(oper, "SLT") == 0) {
        sscanf(command, "%s R%u, R%u, R%u", oper, &rd, &rs, &rt);
        reg[rd] = (reg[rs] < reg[rt]) ? 1 : 0;
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    
    //immediate operations

    if (strcmp(oper, "ADDI") == 0) {
        sscanf(command, "%s R%u, R%u, #%d", oper, &rt, &rs, &immediate);
        reg[rt] = reg[rs] + immediate;
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    
    if (strcmp(oper, "ANDI") == 0) {
        sscanf(command, "%s R%u, R%u, #%d", oper, &rt, &rs, &immediate);
        reg[rt] = reg[rs] & immediate;
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    
    if (strcmp(oper, "ORI") == 0) {
        sscanf(command, "%s R%u, R%u, #%d", oper, &rt, &rs, &immediate);
        reg[rt] = reg[rs] | immediate;
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    
    if (strcmp(oper, "XORI") == 0) {
        sscanf(command, "%s R%u, R%u, #%d", oper, &rt, &rs, &immediate);
        reg[rt] = reg[rs] ^ immediate;
        simulationP(command);
        
        PC += 4;
        
        goto end;
    }
    
    
end:
    cycle_count++;
    
    return;

}

void MIPSsim::simulationP(const char *command)
{
    fprintf(out,HYP);fprintf(out,"\n");
    fprintf(out,"Cycle %d:\t%d\t%s\n\n", cycle_count, PC, command);
    fprintf(out,"Registers\n");
    fprintf(out,"R00:\t");
    fprintf(out, SIMULATION_OUTPUT_FORMAT(reg, 0) );
    fprintf(out,"R08:\t");
    fprintf(out, SIMULATION_OUTPUT_FORMAT(reg, 8) );
    fprintf(out,"R16:\t");
    fprintf(out, SIMULATION_OUTPUT_FORMAT(reg, 16) );
    fprintf(out,"R24:\t");
    fprintf(out, SIMULATION_OUTPUT_FORMAT(reg, 24) );
    fprintf(out,"\nData\n");
    
    int tmp_mem = end_mem - (end_mem - start_mem + 1) % 8;
    
    for(int i = start_mem; i < tmp_mem; i = i + 8)
    {
       fprintf(out, "%d:\t", i * 4);
       fprintf(out, SIMULATION_OUTPUT_FORMAT(mem, i) );
    }
    
    
    if (tmp_mem != end_mem) {
        fprintf(out, "%d:\t", (tmp_mem + 1) * 4);
        for (int i=tmp_mem + 1; i <= end_mem; i++) {
            fprintf(out,"%d\t", mem[i]);
        }
    }
    
    fprintf(out,"\n");
    
    return;
}

/*
main function
*/

int main(int argc, char** argv)
{
    if (argc != 2 ) {
        fprintf(stderr, "Invalid input to [MIPSsim]\n");
        printf("Usage: MIPSsim [input file name]\n");
        return 1;
    }
    
    //input file reader
    FILE *fp = fopen(argv[1], "r");
    
	if(!fp){
        fprintf(stderr, "Can't open input file");
        return 1;
	}
    
    FILE *out_dis = fopen(DISASSEMBLER_OUTPUT_FILE, "w");
    
	char line[INSTRUCTIONS_SIZE + 1];
	bool is_instruction = true;
	int address = INITIAL_COM_ADDR;
    char out[100];
    MIPSsim mipssim;
    map<int, int> inital_mem;
    int data_addr_start = -1;
    int data_addr_end;
	
    
	while(fscanf(fp, "%s\n", &line) != -1){
        
		assert(strlen(line) == INSTRUCTIONS_SIZE);
        
		if(is_instruction){
			char head[3] = {line[0], line[1]};
            
			if(strcmp(head, "01") == 0){
                mipssim.disassemble(1, address, line, out);

                fprintf(out_dis, "%s\n",out);
				if(strcmp(line, BREAK_CODE) == 0)
					is_instruction = false;
			}
			else if (strcmp(head, "11") == 0)
            {
				mipssim.disassemble(2, address, line, out);
                fprintf(out_dis, "%s\n",out);
            }
		}
        //immediate action
		else {
            int num = mipssim.immediateTransform_Signed(line, INSTRUCTIONS_SIZE);
			fprintf(out_dis,"%s\t%d\t%d\n", line, address, num);
            inital_mem[address] = num;
            
            if (data_addr_start == -1) {
                data_addr_start = address;
            }
		}
        
		address += 4;
	}
    
    data_addr_end = address - 4;
    
	fclose(fp);
    fclose(out_dis);
    
    //simulation
    MIPSsim MIPSsim(INITIAL_COM_ADDR, data_addr_start / 4,data_addr_end / 4, SIMULATION_OUTPUT_FILE);
    
    map<int, int>::iterator it;
    for (it = inital_mem.begin(); it != inital_mem.end(); it++) {
        MIPSsim.memory_set(it->first / 4, it->second);
    }
    
    while (MIPSsim.PC != FLAG_COM_ADDR) {
        MIPSsim.execute(mipssim.result[MIPSsim.PC]);
    }
}