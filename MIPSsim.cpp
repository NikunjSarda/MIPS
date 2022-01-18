/*  On my honor, I have neither given nor received unauthorized aid on this assignment */

#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <queue>

using namespace std;

#define FORMATCONSOLEPRINT(dtype,dt) "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",\
dtype[dt], dtype[dt + 1], dtype[dt + 2], dtype[dt + 3], dtype[dt + 4], dtype[dt + 5], dtype[dt + 6], dtype[dt + 7]

class comparecommandsstring {
public:
	bool operator()(const char *s1, const char * s2) {
		if (strcmp(s1, s2) < 0)
			return true;
		else
			return false;
	}
};

class MIPSsimDisassembler{
private:
	map<char*, char*, comparecommandsstring> cat2;
	map<char*, char*, comparecommandsstring> cat1;
    
public:
    map<int, char*> result;
    
public:
	MIPSsimDisassembler();
    ~MIPSsimDisassembler();
	void disassemble(const char* line, int datatype , char* output, int address);
	int TransformSignedImmediate(int length, const char* numbers);
};

class MIPSSimParseInstructionsInformations{
public:
    char* operations;
    unsigned int src1, destination, src2;
    int immediates;
    bool finished;
    
public:
    MIPSSimParseInstructionsInformations(){
        operations = NULL;
        destination = -1;
        src1 = -1;
        src2 = -1;
        immediates = -1;
        finished = false;
    }
    ~MIPSSimParseInstructionsInformations(){
        if(!operations)
            free(operations);
    }
    void instructionsparsing (const char* command);
};
class MIPSSimConsolePipeline {
private:
    FILE *outputFile;
    int classRegister[32], memoryStarts, positionCounter, memoryEnds, classMemory[256];
    MIPSSimConsolePipeline(){};
    void stepInstructionsFetch(MIPSsimDisassembler &MIPSsimDisassembler);
    void stepIssues();
    void stepExecutions();
    void stepMemory();
    void stepWB();
    bool checkSWIssue(int position);
    
    enum FuncUnit{
        None = -1, ALU1, MEM , ALU2
    };
    
public:
    int classinstructionsstalled, classInstructionsLabel, issuePreQueue[4], countsCycles, slotsRemaining, stringExecuted;
    int bufferalu2post[3], classRegisteristerscountswritten[32], bufferpostmemory[3],  instructionsFinished, bufferprememory[3], InstructionsLWFinished;
    bool terminationFlag, stalledFlagIF;
    queue<int> alu1PreQueue;
    vector<char*> Instructions;
    queue<int> alu2PreQueue;
    FuncUnit classRegisterStatus[32];
    vector<MIPSSimParseInstructionsInformations> instructionsinformations;
    MIPSSimConsolePipeline(const char* localFile, int localPositionCounter, int localMemoryStart, int localMemoryEnd);
    ~MIPSSimConsolePipeline();
    void pipelineSimulationPrint();
    void pipelineProcess(MIPSsimDisassembler &MIPSsimDisassembler);
    inline void setMemory(int value, int address);
    void showInit();
    bool checkHazardWAR(int instructionslabel, unsigned int rs);
    void delIPQ(int position);
    bool checkHazardRAW(int instructionslabel, unsigned int rs);
    int sizeIPQ();
    bool checkHazardWAW(int instructionslabel, unsigned int rs);
    void reorgIPQ();
    void addIPQ(int instructionslabel);

};
MIPSsimDisassembler::MIPSsimDisassembler(){
	cat1["0000"] = "J";
	cat2["0000"] = "ADD";
    cat1["0001"] = "JR";
	cat2["0001"] = "SUB";
    cat1["0010"] = "BEQ";
	cat2["0110"] = "NOR";
    cat1["0011"] = "BLTZ";
	cat2["1001"] = "ANDI";
    cat1["0100"] = "BGTZ";
	cat2["1000"] = "ADDI";
    cat1["0101"] = "BREAK";
	cat2["1010"] = "ORI";
    cat1["0110"] = "SW";
	cat2["0111"] = "SLT";
    cat1["0111"] = "LW";
	cat2["0101"] = "XOR";
    cat1["1000"] = "SLL";
	cat2["0100"] = "OR";
    cat1["1001"] = "SRL";
	cat2["0011"] = "AND";
    cat1["1010"] = "SRA";
	cat2["1011"] = "XORI";
    cat1["1011"] = "NOP";
    cat2["0010"] = "MUL";
}
MIPSsimDisassembler::~MIPSsimDisassembler(){
    map<int, char*>::iterator it;
    for(it = result.begin(); it != result.end(); it++)
    {
        free(it->second);
    }
}

void MIPSsimDisassembler::disassemble(const char* line, int datatype, char* output, int address){
	char operationscodes[5],mapoperationscodes[10];
	unsigned int instructions, sa, rs, rt, rd, base;
    int negate, immediate;
    char* commands = (char*)malloc(sizeof(char)*30);
    
    memcpy(operationscodes, line + sizeof(char)*2, sizeof(char)*4);
    operationscodes[4]='\0';
    
	if(datatype == 1){
        strcpy(mapoperationscodes, cat1[operationscodes]);
		if(strcmp(mapoperationscodes,"J") == 0){
			char stringinstructionsindexes[27];
			memcpy(stringinstructionsindexes, line + sizeof(char)*6, sizeof(char)*27);
			instructions = strtol(stringinstructionsindexes, NULL,2) << 2;
			sprintf(output, "%s\t%d\t%s #%u", line, address, mapoperationscodes, instructions);
            sprintf(commands, "%s #%u", mapoperationscodes, instructions);
            result[address] = commands;
            return;
		}
        
        if (strcmp(mapoperationscodes, "JR") == 0) {
            char stringRS[6];
            memcpy(stringRS, line + sizeof(char)*6, sizeof(char)*5);
            stringRS[5]='\0';
            
            rs = strtol(stringRS, NULL, 2);
            
            sprintf(output, "%s\t%d\t%s R%u", line, address, mapoperationscodes, rs);
            sprintf(commands, "%s R%u", mapoperationscodes, rs);
            result[address] = commands;
            return;
        }
        
        if (strcmp(mapoperationscodes, "BEQ")==0) {
            char stringRS[6],stringRT[6], stringnegate[17];
            memcpy(stringRS, line + sizeof(char)*6, sizeof(char)*5);
            memcpy(stringRT, line + sizeof(char)*11, sizeof(char)*5);
            memcpy(stringnegate, line + sizeof(char)*16, sizeof(char)*17);
            stringRT[5]='\0';
            stringRS[5]='\0';
            rs = strtol(stringRS, NULL, 2);
            rt = strtol(stringRT, NULL, 2);
            negate = strtol(stringnegate, NULL,2) << 2;
            
            sprintf(output, "%s\t%d\t%s R%u, R%u, #%d", line, address, mapoperationscodes, rs, rt, negate);
            sprintf(commands, "%s R%u, R%u, #%d", mapoperationscodes, rs, rt, negate);
            result[address] = commands;
            
            return;
        }
        
        if (strcmp(mapoperationscodes, "BLTZ") == 0 || strcmp(mapoperationscodes, "BGTZ") == 0) {
            char stringRS[6],stringnegate[17];
            memcpy(stringRS, line + sizeof(char)*6, sizeof(char)*5);
            memcpy(stringnegate, line + sizeof(char)*16, sizeof(char)*17);
            stringRS[5]='\0';
            
            rs = strtol(stringRS, NULL, 2);
            negate = strtol(stringnegate, NULL, 2) << 2;
            
            sprintf(output, "%s\t%d\t%s R%u, #%d", line, address, mapoperationscodes, rs, negate);
            sprintf(commands, "%s R%u, #%d", mapoperationscodes, rs, negate);
            result[address] = commands;
            return;
        }
        
        if (strcmp(mapoperationscodes, "BREAK") == 0 || strcmp(mapoperationscodes, "NOP") == 0) {
            sprintf(output, "%s\t%d\t%s", line, address, mapoperationscodes);
            sprintf(commands, "%s", mapoperationscodes);
            result[address] = commands;
            return;
        }
        
        if (strcmp(mapoperationscodes, "SW") == 0 || strcmp(mapoperationscodes, "LW") == 0 ) {
            char stringBase[6], stringRT[6], stringnegate[17];
            memcpy(stringBase, line + sizeof(char)*6, sizeof(char)*5);
            memcpy(stringRT, line + sizeof(char)*11, sizeof(char)*5);
            memcpy(stringnegate, line + sizeof(char)*16, sizeof(char)*17);
            stringRT[5]='\0';
            stringBase[5]='\0';
            
            base = strtol(stringBase, NULL, 2);
            rt = strtol(stringRT, NULL, 2);
            negate = strtol(stringnegate, NULL, 2);
            
            sprintf(output, "%s\t%d\t%s R%u, %d(R%u)",line, address, mapoperationscodes, rt, negate, base);
            sprintf(commands, "%s R%u, %d(R%u)",mapoperationscodes,rt,negate,base);
            result[address] = commands;
            return;
        }
        
        if (strcmp(mapoperationscodes, "SLL") == 0 || strcmp(mapoperationscodes, "SRL") == 0 ||
            strcmp(mapoperationscodes, "SRA") == 0) {
            char stringRT[6], stringRD[6], stringSA[6];
            memcpy(stringRT, line + sizeof(char)*11, sizeof(char)*5);
            memcpy(stringRD, line + sizeof(char)*16, sizeof(char)*5);
            memcpy(stringSA, line + sizeof(char)*21, sizeof(char)*5);
            stringSA[5]='\0';
            stringRD[5]='\0';
            stringRT[5]='\0';
            rt = strtol(stringRT, NULL, 2);
            rd = strtol(stringRD, NULL, 2);
            sa = strtol(stringSA, NULL, 2);
            
            sprintf(output, "%s\t%d\t%s R%u, R%u, #%u", line, address, mapoperationscodes, rd, rt, sa);
            sprintf(commands, "%s R%u, R%u, #%u", mapoperationscodes, rd, rt, sa);
            result[address] = commands;
            return;
        }
	}
	else
	{
        strcpy(mapoperationscodes, cat2[operationscodes]);

		if(strcmp(mapoperationscodes, "ADD") == 0 || strcmp(mapoperationscodes, "SUB") == 0 ||
           strcmp(mapoperationscodes, "MUL") == 0 || strcmp(mapoperationscodes, "AND") == 0 ||
           strcmp(mapoperationscodes, "OR") == 0 || strcmp(mapoperationscodes, "XOR") == 0 ||
           strcmp(mapoperationscodes, "NOR") == 0 || strcmp(mapoperationscodes, "SLT") == 0){
			
			char stringRS[6],stringRT[6],stringRD[6];
            memcpy(stringRD, line + sizeof(char)*16, sizeof(char)*5);
            memcpy(stringRT, line + sizeof(char)*11, sizeof(char)*5);
			memcpy(stringRS, line + sizeof(char)*6, sizeof(char)*5);
			
            stringRD[5]='\0';
            stringRT[5]='\0';
            stringRS[5]='\0';
            rd = strtol(stringRD,NULL,2);
            rt = strtol(stringRT,NULL,2);
			rs = strtol(stringRS,NULL,2);
            
			sprintf(output, "%s\t%d\t%s R%u, R%u, R%u", line, address, mapoperationscodes, rd, rs, rt);
            sprintf(commands, "%s R%u, R%u, R%u", mapoperationscodes, rd, rs, rt);
            result[address] = commands;
            return;
		}
        
		if(strcmp(mapoperationscodes, "ADDI") == 0 || strcmp(mapoperationscodes, "ANDI") == 0||
           strcmp(mapoperationscodes, "ORI") == 0 || strcmp(mapoperationscodes, "XORI") == 0){
			char stringNumber[17];
			char stringRS[6],stringRT[6];
			memcpy(stringRT, line+sizeof(char)*11, sizeof(char)*5);
            memcpy(stringRS, line + sizeof(char)*6, sizeof(char)*5);
            stringRT[5]='\0';
            stringRS[5]='\0';
            rt = strtol(stringRT,NULL,2);
			rs = strtol(stringRS,NULL,2);
			
            
			memcpy(stringNumber, line+sizeof(char)*16, sizeof(char)*17);
			
			immediate = TransformSignedImmediate(16, stringNumber);
            
			sprintf(output, "%s\t%d\t%s R%u, R%u, #%d",line,address,mapoperationscodes,rt,rs,immediate);
            sprintf(commands, "%s R%u, R%u, #%d",mapoperationscodes,rt,rs,immediate);
            result[address] = commands;
            return;
		}
	}
    
}

int MIPSsimDisassembler::TransformSignedImmediate(int length, const char* numbers){
	if(numbers[0] == '0')
		return strtol(numbers, NULL, 2);
	else{
		char* left = (char*)malloc(sizeof(char)*length);
		memcpy(left, numbers + sizeof(char)*1, sizeof(char)*length);
		int immediate = strtol(left, NULL, 2) - (1 << 31);
        
		free(left);
        
		return immediate;
	} 
}
void MIPSSimParseInstructionsInformations::instructionsparsing (const char* command){
    char* operation = (char*)malloc(sizeof(char)*10);
    unsigned int rs, sa, rt, base, rd;
    int negate, immediate;
    sscanf(command, "%s", operation);
        
    operations = operation;
    finished = false;
    if (strcmp(operations,"J") == 0 || strcmp(operations, "JR") == 0 ||strcmp(operations, "BEQ") == 0 ||
        strcmp(operations, "BLTZ") == 0 || strcmp(operations, "BGTZ") == 0 ||
        strcmp(operations, "NOP") == 0 || strcmp(operations, "BREAK") == 0) {
        return;
    }
    if (strcmp(operations, "LW") == 0) {
        sscanf(command, "%s R%u, %d(R%u)", operations, &rt, &negate, &base);
        destination = rt;
        src1 = base;
        immediates = negate;
        return;
    }
    if (strcmp(operations, "SW") == 0) {
        sscanf(command, "%s R%u, %d(R%u)", operations, &rt, &negate, &base);
        src1 = base;
        src2 = rt;
        immediates = negate;
        return;
    }
    if (strcmp(operations, "SLL") == 0 || strcmp(operations, "SRL") == 0 || strcmp(operations, "SRA") == 0) {
        sscanf(command, "%s R%u, R%u, #%u", operations, &rd, &rt, &sa);
        destination = rd;
        src1 = rt;
        immediates = sa;
        return;
    }
    if(strcmp(operations, "ADD") == 0 || strcmp(operations, "SUB") == 0 ||
        strcmp(operations, "MUL") == 0 || strcmp(operations, "AND") == 0 ||
        strcmp(operations, "OR") == 0 || strcmp(operations, "XOR") == 0 ||
        strcmp(operations, "NOR") == 0 || strcmp(operations, "SLT") == 0){
        sscanf(command, "%s R%u, R%u, R%u", operations, &rd, &rs, &rt);
        destination = rd;
        src1 = rs;
        src2 = rt;
        return;
    }
    if(strcmp(operations, "ADDI") == 0 || strcmp(operations, "ANDI") == 0||
        strcmp(operations, "ORI") == 0 || strcmp(operations, "XORI") == 0){
        sscanf(command, "%s R%u, R%u, #%d", operations, &rt, &rs, &immediate);
        destination = rt;
        src1 = rs;
        immediates = immediate;
        return;
    }
}
inline void MIPSSimConsolePipeline::setMemory(int value, int address){
    classMemory[address] = value;
}
    
void MIPSSimConsolePipeline::showInit()
{
    printf("classRegister SHOW\n");
    for (int i = 0; i < 32; i++) {
        printf("R[%d]=%d ", i, classRegister[i]);
    }
    printf("\n");
}
    
bool MIPSSimConsolePipeline::checkHazardWAR(int instructionslabel, unsigned int rs){
    assert(instructionslabel < instructionsinformations.size());
    for (int i = 0; i < instructionslabel; i++) {
        if(instructionsinformations[i].finished == false &&
            (instructionsinformations[i].src2 == rs || instructionsinformations[i].src1 == rs))
            return true;
        }
    return false;
}
bool MIPSSimConsolePipeline::checkHazardRAW(int instructionslabel, unsigned int rs){
    for (int i = 0; i < instructionslabel; i++) {
        if (instructionsinformations[i].finished == false && instructionsinformations[i].destination == rs) {
            return true;
        }
    }
    return false;
}
bool MIPSSimConsolePipeline::checkHazardWAW(int instructionslabel, unsigned int rs){
    assert(instructionslabel < instructionsinformations.size());
    for (int i = 0; i < instructionslabel; i++) {
        if (instructionsinformations[i].destination == rs && instructionsinformations[i].finished == false) {
            return true;
        }
    }
    return false;
}
void MIPSSimConsolePipeline::delIPQ(int position){
    assert(position > -1  && position < 4);
    assert(issuePreQueue[position] != -1);
    issuePreQueue[position] = -1;
}
void MIPSSimConsolePipeline::addIPQ(int instructionslabel){
    for (int i = 0; i < 4; i++) {
        if (issuePreQueue[i] == -1) {
            issuePreQueue[i] = instructionslabel;
            break;
        }
    }
}
int MIPSSimConsolePipeline::sizeIPQ(){
        int size = 0;
    for (int i = 0; i < 4; i++) {
        if (issuePreQueue[i] != -1) {
            size++;
        }
    }
    return size;
}
void MIPSSimConsolePipeline::reorgIPQ(){
    int tcopy[4], currindex = 0;
    memcpy(tcopy, issuePreQueue, sizeof(int)*4);
    for (int i = 0; i < 4; i++) {
        assert(issuePreQueue[i] == tcopy[i]);
    }
    for (int i = 0; i < 4; i++) {
        if (tcopy[i] != -1) {
            issuePreQueue[currindex] = tcopy[i];
            currindex++;
        }
    }
        
    for (int i = currindex; i < 4; i++) {
        issuePreQueue[i] = -1;
    }
}


MIPSSimConsolePipeline::MIPSSimConsolePipeline(const char* localFile, int localPositionCounter, int localMemoryStart, int localMemoryEnds)
:positionCounter(localPositionCounter), memoryStarts(localMemoryStart), memoryEnds(localMemoryEnds)
{
    countsCycles = 1;
    classinstructionsstalled = -1;
    stringExecuted = -1;
    terminationFlag = false;
    stalledFlagIF = false;
    classInstructionsLabel = 0;
    instructionsFinished = -1;
    InstructionsLWFinished = -1;
    
    slotsRemaining = 4;
    memset(classRegister, 0, sizeof(unsigned int)*32);
    memset(classMemory, 0, sizeof(int)*256);
    memset(classRegisterStatus, None, sizeof(int)*32);
    memset(issuePreQueue, -1, sizeof(int)*4);
    memset(bufferalu2post, -1, sizeof(int)*3);
    memset(bufferprememory, -1, sizeof(int)*3);
    memset(bufferpostmemory, -1, sizeof(int)*3);
    memset(classRegisteristerscountswritten, 0 , sizeof(int)*32);
    
    outputFile = fopen(localFile, "w");
}

MIPSSimConsolePipeline::~MIPSSimConsolePipeline()
{
    if (!outputFile) {
        fclose(outputFile);
    }
}


void MIPSSimConsolePipeline::pipelineProcess(MIPSsimDisassembler &MIPSsimDisassembler){
    stepWB();
    stepMemory();
    stepExecutions();
    stepIssues();
    stepInstructionsFetch(MIPSsimDisassembler);
}
bool MIPSSimConsolePipeline::checkSWIssue(int position){
    for (int i = 0; i < position; i++) {
        if (strcmp(instructionsinformations[issuePreQueue[i]].operations, "SW") ==0) {
            return true;
        }
    }
    return false;
}

void MIPSSimConsolePipeline::stepInstructionsFetch(MIPSsimDisassembler &MIPSsimDisassembler)
{
    int currNumberIF = 0;
    unsigned int localInstructions, rs,rt;
    char operations[10];
    char* command = NULL;
    int negate;
    
    if (!terminationFlag){
        
        if (classinstructionsstalled != -1) {
            assert(stalledFlagIF == true);
            command = Instructions[classinstructionsstalled];
            sscanf(command, "%s", operations);
            if(strcmp(operations, "BGTZ") == 0){
                sscanf(command, "%s R%u, #%d", operations, &rs, &negate);
                if(classRegisterStatus[rs] == None){
                    stalledFlagIF = false;
                    stringExecuted = classinstructionsstalled;
                    positionCounter = (classRegister[rs] < 0) ? (positionCounter + 4) : (positionCounter + negate + 4);
                    classinstructionsstalled = -1;
                }
            }
            else if (strcmp(operations, "BLTZ") == 0){
                sscanf(command, "%s R%u, #%d", operations, &rs, &negate);
                if(classRegisterStatus[rs] == None){
                    stalledFlagIF = false;
                    positionCounter = (classRegister[rs] > 0) ? (positionCounter + 4) : (positionCounter + negate + 4);
                    stringExecuted = classinstructionsstalled;
                    classinstructionsstalled = -1;
                }
            }
            else if (strcmp(operations, "BEQ") == 0) {
                sscanf(command, "%s R%u, R%u, #%d", operations, &rs, &rt, &negate);
                if (classRegisterStatus[rs] == None && classRegisterStatus[rt] == None) {
                    stalledFlagIF = false;
                    stringExecuted = classinstructionsstalled;
                    positionCounter = (classRegister[rs] == classRegister[rt]) ? (positionCounter + negate + 4) : (positionCounter + 4);
                    classinstructionsstalled = -1;
                }
            }
            else if (strcmp(operations, "JR") == 0) {
                sscanf(command, "%s R%u", operations, &rs);
                if (classRegisterStatus[rs] == None) {
                    stalledFlagIF = false;
                    stringExecuted = classinstructionsstalled;
                    classinstructionsstalled = -1;
                    positionCounter = classRegister[rs];
                }
            }
        }
        else{
            stringExecuted = -1;
            while (currNumberIF < slotsRemaining && currNumberIF < 2) {
                command = MIPSsimDisassembler.result[positionCounter];
                sscanf(command, "%s", operations);
                
                Instructions.push_back(command);
                MIPSSimParseInstructionsInformations parseInstructionsInformations;
                parseInstructionsInformations.instructionsparsing(command);
                instructionsinformations.push_back(parseInstructionsInformations);
                if (strcmp(operations, "BGTZ") == 0){
                    sscanf(command, "%s R%u, #%d", operations, &rs, &negate);
                    if(classRegisterStatus[rs] != -1){
                        stalledFlagIF = true;
                        classinstructionsstalled = classInstructionsLabel;
                    } else{
                        positionCounter = (classRegister[rs] > 0) ? (positionCounter + 4 + negate) : (positionCounter + 4);
                        stringExecuted = classInstructionsLabel;
                    }
                    classInstructionsLabel++;
                    break;
                }
                else if(strcmp(operations, "JR") == 0){
                    sscanf(command, "%s R%u", operations, &rs);
                    if (classRegisterStatus[rs] != None) {
                        stalledFlagIF = true;
                        classinstructionsstalled = classInstructionsLabel;
                    }else{
                        positionCounter = classRegister[rs];
                        stringExecuted = classInstructionsLabel;
                    }
                    classInstructionsLabel++;
                    break;
                }
                else if (strcmp(operations, "BEQ") == 0){
                    sscanf(command, "%s R%u, R%u, #%d", operations, &rs, &rt, &negate);
                    if (classRegisterStatus[rs] != None || classRegisterStatus[rt] != None) {
                        stalledFlagIF = true;
                        classinstructionsstalled = classInstructionsLabel;
                    } else{
                        positionCounter = (classRegister[rs] != classRegister[rt]) ? (positionCounter + 4) : (positionCounter + 4 + negate);
                        stringExecuted = classInstructionsLabel;
                    }
                    classInstructionsLabel++;
                    break;
                }
                else if (strcmp(operations, "J") == 0) {
                    sscanf(command, "%s #%u", operations, &localInstructions);
                    positionCounter = localInstructions;
                    stringExecuted = classInstructionsLabel;
                    classInstructionsLabel++;
                    break;
                }
                else if (strcmp(operations, "BREAK") == 0){
                    stringExecuted = classInstructionsLabel;
                    classInstructionsLabel++;
                    terminationFlag = true;
                    break;
                }
                else if (strcmp(operations, "BLTZ") == 0){
                    sscanf(command, "%s R%u, #%d", operations, &rs, &negate);
                    if(classRegisterStatus[rs] != -1){
                        stalledFlagIF = true;
                        classinstructionsstalled = classInstructionsLabel;
                    } else{
                        positionCounter = (classRegister[rs] < 0) ? (positionCounter + 4 + negate) : (positionCounter + 4);
                        stringExecuted = classInstructionsLabel;
                    }
                    classInstructionsLabel++;
                    break;
                }
                else if (strcmp(operations, "NOP") == 0){
                    stringExecuted = classInstructionsLabel;
                    positionCounter += 4;
                    classInstructionsLabel++;
                }
                else{
                    addIPQ(classInstructionsLabel);
                    classInstructionsLabel++;
                    positionCounter += 4;
                }
                currNumberIF++;
            }
            
        }
        for (int i = 0; i < 32; i++) {
            if (classRegisteristerscountswritten[i] == 1) {
                classRegisterStatus[i] = None;
                classRegisteristerscountswritten[i] = 0;
                assert(instructionsFinished != -1 || InstructionsLWFinished != -1);
                if (instructionsFinished != -1) {
                    instructionsinformations[instructionsFinished].finished = true;
                }
                if (InstructionsLWFinished != -1) {
                    instructionsinformations[InstructionsLWFinished].finished = true;
                }
                
            }
        }
        slotsRemaining = 4 - sizeIPQ();
    }
}
void MIPSSimConsolePipeline::stepIssues(){
    char* command = NULL;
    char operations[10];
    int size = sizeIPQ();
    bool issuedMemoeyFlag = false;
    bool issuedALUFlag = false;
    
    for (int i = 0; i < size; i++){
        int instructionslabel = issuePreQueue[i];
        command = Instructions[instructionslabel];
        sscanf(command, "%s", operations);
        if(!issuedALUFlag &&
           (strcmp(operations, "SLL") == 0 || strcmp(operations, "SRL") == 0 || strcmp(operations, "SRA") == 0 ||
            strcmp(operations, "ADDI") == 0 || strcmp(operations, "ANDI") == 0 ||
            strcmp(operations, "ORI") == 0 || strcmp(operations, "XORI") == 0)){
               if (alu2PreQueue.size() < 2) {
                   int classRegisterReader = instructionsinformations[instructionslabel].src1;
                   int destination = instructionsinformations[instructionslabel].destination;
                   assert(destination != -1 && classRegisterReader != -1);
                   if (!checkHazardRAW(instructionslabel, classRegisterReader) &&
                       !checkHazardWAW(instructionslabel, destination) && !checkHazardWAR(instructionslabel, destination)) {
                       classRegisterStatus[destination]=ALU2;
                       delIPQ(i);
                       alu2PreQueue.push(instructionslabel);
                       issuedALUFlag = true;
                   }
               }
            continue;
        }
        if (!issuedMemoeyFlag && strcmp(operations, "SW") == 0) {
            if (alu1PreQueue.size() < 2) {
                int classRegister1Reader = instructionsinformations[instructionslabel].src1;
                int classRegister2Reader = instructionsinformations[instructionslabel].src2;
                if (!checkHazardRAW(instructionslabel, classRegister1Reader) &&
                    !checkHazardRAW(instructionslabel, classRegister2Reader) && !checkSWIssue(i)) {
                    delIPQ(i);
                    alu1PreQueue.push(instructionslabel);
                    issuedMemoeyFlag = true;
                }
            }
            continue;
        }
        if (!issuedMemoeyFlag && strcmp(operations, "LW") == 0) {
            if (alu1PreQueue.size() < 2) {
                int destination = instructionsinformations[instructionslabel].destination;
                int classRegisterReader = instructionsinformations[instructionslabel].src1;
                if (!checkHazardRAW(instructionslabel, classRegisterReader) &&
                    !checkHazardWAW(instructionslabel, destination) &&
                    !checkHazardWAR(instructionslabel, destination) && !checkSWIssue(i)) {
                        issuedMemoeyFlag = true;
                        delIPQ(i);
                        alu1PreQueue.push(instructionslabel);
                }
                
            }
            continue;
        }
        if(!issuedALUFlag &&
           (strcmp(operations, "ADD") == 0 || strcmp(operations, "SUB") == 0 ||
            strcmp(operations, "MUL") == 0 || strcmp(operations, "AND") == 0 ||
            strcmp(operations, "OR") == 0 || strcmp(operations, "XOR") == 0 ||
            strcmp(operations, "NOR") == 0 || strcmp(operations, "SLT") == 0)){
               if (alu2PreQueue.size() < 2) {
                   int destination = instructionsinformations[instructionslabel].destination;
                   int classRegister1Reader = instructionsinformations[instructionslabel].src1;
                   int classRegister2Reader = instructionsinformations[instructionslabel].src2;
                   assert(destination !=-1 && classRegister1Reader != -1 && classRegister2Reader != -1);
                   
                   if (!checkHazardRAW(instructionslabel, classRegister1Reader) && !checkHazardWAW(instructionslabel, destination)
                        && !checkHazardRAW(instructionslabel, classRegister2Reader)
                        && !checkHazardWAR(instructionslabel, destination)) {
                       classRegisterStatus[destination] = ALU2;
                       delIPQ(i);
                       alu2PreQueue.push(instructionslabel);
                       issuedALUFlag = true;
                   }
               } 
            continue;
        }
    }
    reorgIPQ();
}
void MIPSSimConsolePipeline::stepExecutions(){
    if (!alu1PreQueue.empty()) {
        int instructionslabel = alu1PreQueue.front();
        bufferprememory[0] = instructionslabel;
        char *operations = instructionsinformations[instructionslabel].operations;
        alu1PreQueue.pop();
        if (strcmp(operations, "SW") == 0) {
            bufferprememory[1] = instructionsinformations[instructionslabel].src2;
            bufferprememory[2] = (classRegister[instructionsinformations[instructionslabel].src1] + instructionsinformations[instructionslabel].immediates)/4;
            instructionsinformations[instructionslabel].finished = true;
            goto ALU2;
        }
        if (strcmp(operations, "LW") == 0) {
            bufferprememory[1] = instructionsinformations[instructionslabel].destination;
            bufferprememory[2] = (classRegister[instructionsinformations[instructionslabel].src1] + instructionsinformations[instructionslabel].immediates)/4;
            goto ALU2;
        }
    }
ALU2:
    if (!alu2PreQueue.empty()) {
        
        int instructionslabel = alu2PreQueue.front(); alu2PreQueue.pop();
        bufferalu2post[0] = instructionslabel;
        bufferalu2post[1] = instructionsinformations[instructionslabel].destination;
        int classRegister1Reader, classRegister2Reader, localImmediates;
        char *operations = instructionsinformations[instructionslabel].operations;
        assert(operations != NULL);
        if (strcmp(operations, "MUL") == 0) {
            classRegister2Reader = instructionsinformations[instructionslabel].src2;
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            bufferalu2post[2] = classRegister[classRegister1Reader] * classRegister[classRegister2Reader];
            return;
        }
        if (strcmp(operations, "ADD") == 0) {
            classRegister2Reader = instructionsinformations[instructionslabel].src2;
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            bufferalu2post[2] = classRegister[classRegister1Reader] + classRegister[classRegister2Reader];
            return;
        }
        if (strcmp(operations, "AND") == 0) {
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            classRegister2Reader = instructionsinformations[instructionslabel].src2;
            bufferalu2post[2] = classRegister[classRegister1Reader]&classRegister[classRegister2Reader];
            return;
        }
        if (strcmp(operations, "XORI") == 0) {
            localImmediates = instructionsinformations[instructionslabel].immediates;
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            bufferalu2post[2] = classRegister[classRegister1Reader] ^ localImmediates;
            return;
        }
        if (strcmp(operations, "SUB") == 0) {
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            classRegister2Reader = instructionsinformations[instructionslabel].src2;
            bufferalu2post[2] = classRegister[classRegister1Reader] - classRegister[classRegister2Reader];
            return;
        }
        if (strcmp(operations, "XOR") == 0) {
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            classRegister2Reader = instructionsinformations[instructionslabel].src2;
            bufferalu2post[2] = classRegister[classRegister1Reader]^classRegister[classRegister2Reader];
            return;
        }
        if (strcmp(operations, "OR") == 0) {
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            classRegister2Reader = instructionsinformations[instructionslabel].src2;
            bufferalu2post[2] = classRegister[classRegister1Reader]|classRegister[classRegister2Reader];
            return;
        }
        if (strcmp(operations, "SLL") == 0) {
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            localImmediates = instructionsinformations[instructionslabel].immediates;
            bufferalu2post[2] = classRegister[classRegister1Reader] << localImmediates;
            return;
        }
        if (strcmp(operations, "NOR") == 0) {
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            classRegister2Reader = instructionsinformations[instructionslabel].src2;
            bufferalu2post[2] = ~(classRegister[classRegister1Reader]|classRegister[classRegister2Reader]);
            return;
        }
        if (strcmp(operations, "ADDI") == 0) {
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            localImmediates = instructionsinformations[instructionslabel].immediates;
            bufferalu2post[2] = classRegister[classRegister1Reader] + localImmediates;
            return;
        }
        if (strcmp(operations, "SRL") == 0) {
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            localImmediates = instructionsinformations[instructionslabel].immediates;
            bufferalu2post[2] = (unsigned)classRegister[classRegister1Reader] >> localImmediates;
            return;
        }
        if (strcmp(operations, "ORI") == 0) {
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            localImmediates = instructionsinformations[instructionslabel].immediates;
            bufferalu2post[2] = classRegister[classRegister1Reader] | localImmediates;
            return;
        }
        if (strcmp(operations, "SRA") == 0) {
            localImmediates = instructionsinformations[instructionslabel].immediates;
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            bufferalu2post[2] = classRegister[classRegister1Reader] >> localImmediates;
            return;
        }
        if (strcmp(operations, "ANDI") == 0) {
            classRegister1Reader = instructionsinformations[instructionslabel].src1;
            localImmediates = instructionsinformations[instructionslabel].immediates;
            bufferalu2post[2] = classRegister[classRegister1Reader] & localImmediates;
            return;
        }
    }
}
void MIPSSimConsolePipeline::stepMemory(){
    if (bufferprememory[0] != -1) {
        int instructionslabel = bufferprememory[0];
        if (strcmp(instructionsinformations[instructionslabel].operations,"LW") == 0) {
            bufferpostmemory[1] = bufferprememory[1];
            bufferpostmemory[0] = bufferprememory[0];
            bufferpostmemory[2] = classMemory[bufferprememory[2]];
        }
        else{
            assert(strcmp(instructionsinformations[instructionslabel].operations,"SW") == 0);
            classMemory[bufferprememory[2]] = classRegister[bufferprememory[1]];
            instructionsinformations[instructionslabel].finished = true;
        }
        bufferprememory[0] = -1;
    }
}
void MIPSSimConsolePipeline::stepWB(){
    
    if (bufferpostmemory[0] != -1) {
        int instructionslabel = bufferpostmemory[0];
        int destination = bufferpostmemory[1];
        
        classRegister[destination] = bufferpostmemory[2];
        
        assert(classRegisteristerscountswritten[destination] == 0);
        classRegisteristerscountswritten[destination] = 1;
        InstructionsLWFinished = instructionslabel;
        
        bufferpostmemory[0] = -1;
    }
    
    if (bufferalu2post[0] != -1) {
        int instructionslabel = bufferalu2post[0];
        int destination = bufferalu2post[1];
        
        classRegister[destination] = bufferalu2post[2];
        
        assert(classRegisteristerscountswritten[destination] == 0);
        classRegisteristerscountswritten[destination] = 1;
        instructionsFinished = instructionslabel;
        
        bufferalu2post[0] = -1;
    }    
}
void MIPSSimConsolePipeline::pipelineSimulationPrint()
{
    fprintf(outputFile, "--------------------");
    fprintf(outputFile, "\n");
    fprintf(outputFile, "Cycle %d:\n\n", countsCycles);
    fprintf(outputFile, "IF Unit:\n");
    if (classinstructionsstalled != -1) {
        fprintf(outputFile, "\tWaiting Instruction:");
        fprintf(outputFile," [%s]\n", Instructions[classinstructionsstalled]);
    }
    else{
        fprintf(outputFile, "\tWaiting Instruction: ");
        fprintf(outputFile,"\n");
    }
    if (stringExecuted != -1) {
        fprintf(outputFile,"\tExecuted Instruction:");
        fprintf(outputFile, " [%s]\n", Instructions[stringExecuted]);
    }
    else{
        fprintf(outputFile,"\tExecuted Instruction: ");
        fprintf(outputFile, "\n");
    }
    fprintf(outputFile,"Pre-Issue Queue:\n");
    int size = sizeIPQ();
    for (int i = 0; i < size; i++) {
        fprintf(outputFile, "\tEntry %d: [%s]\n", i, Instructions[issuePreQueue[i]]);
    }
    for (int i = size; i < 4; i++) {
        fprintf(outputFile, "\tEntry %d:\n", i);
    }
    int n = 0;
    queue<int> preQueuealu1temporary(alu1PreQueue);
    fprintf(outputFile, "Pre-ALU1 Queue:\n");
    while (!preQueuealu1temporary.empty()) {
        fprintf(outputFile, "\tEntry %d: [%s]\n", n, Instructions[preQueuealu1temporary.front()]);
        preQueuealu1temporary.pop();
        n++;
    }
    for (int k = n; k < 2; k++) {
        fprintf(outputFile, "\tEntry %d:\n", k);
    }
    n = 0;
    fprintf(outputFile, "Pre-MEM Queue:");
    if (bufferprememory[0] != -1) {
        fprintf(outputFile," [%s]\n", Instructions[bufferprememory[0]]);
    }
    else{
        fprintf(outputFile, "\n");
    }
    fprintf(outputFile, "Post-MEM Queue:");
    if (bufferpostmemory[0] != -1) {
        fprintf(outputFile, " [%s]\n", Instructions[bufferpostmemory[0]]);
    }
    else{fprintf(outputFile, "\n");
    }
    queue<int> preQueuealu2Temporary(alu2PreQueue);
    fprintf(outputFile, "Pre-ALU2 Queue:\n");
    
    while (!preQueuealu2Temporary.empty()) {
        fprintf(outputFile,"\tEntry %d: [%s]\n", n, Instructions[preQueuealu2Temporary.front()]);
        preQueuealu2Temporary.pop();
        n++;
    }
    for (int k = n; k < 2; k++) {
        fprintf(outputFile,"\tEntry %d:\n", k);
    }
    fprintf(outputFile,"Post-ALU2 Queue:");
    if (bufferalu2post[0] != -1) {
        fprintf(outputFile, " [%s]\n\n", Instructions[bufferalu2post[0]]);
    }
    else{fprintf(outputFile, "\n\n");
    }
    fprintf(outputFile, "Registers\n");
    fprintf(outputFile, "R00:\t");
    fprintf(outputFile, FORMATCONSOLEPRINT(classRegister, 0));
    fprintf(outputFile, "R08:\t");
    fprintf(outputFile, FORMATCONSOLEPRINT(classRegister, 8));
    fprintf(outputFile, "R16:\t");
    fprintf(outputFile, FORMATCONSOLEPRINT(classRegister, 16));
    fprintf(outputFile, "R24:\t");
    fprintf(outputFile, FORMATCONSOLEPRINT(classRegister, 24));
    fprintf(outputFile, "\n");
    fprintf(outputFile, "Data\n");
    int memorytemperory = memoryEnds - (memoryEnds - memoryStarts + 1) % 8;
    
    for(int i = memoryStarts;  i < memorytemperory; i = i + 8)
    {
        fprintf(outputFile, "%d:\t", i*4);
        fprintf(outputFile, FORMATCONSOLEPRINT(classMemory, i) );
    }
    
    if (memorytemperory != memoryEnds) {
        fprintf(outputFile, "%d:\t", (memorytemperory + 1)*4);
        for (int i = memorytemperory + 1; i <= memoryEnds; i++) {
            fprintf(outputFile, "%d\t", classMemory[i]);
        }
    }   
}
int main(int argc, char** argv)
{
    if (argc != 2 ) {
        fprintf(stderr, "Invalid input to [MIPSsim]\n");
        printf("Usage: MIPSsim [input file name]\n");
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
	if(!f){
        fprintf(stderr, "Can't open input file");
        return 1;
	}
    FILE *DO = fopen("disassembly.txt", "w");
	bool instructionsFlag = true;
    char output[100], line[33];
	MIPSsimDisassembler MIPSsimDisassembler;
    map<int,int> memoryInitial;
    int addressDataStart = -1, address = 256, addressDataEnds;
	
    
	while(fscanf(f, "%s\n", &line) != -1){
        
		assert(strlen(line) == 32);
        
		if(instructionsFlag){
			char header[3] = {line[0], line[1]};
            
			if(strcmp(header, "01") == 0){
                MIPSsimDisassembler.disassemble(line, 1, output, address);
                
                fprintf(DO, "%s\n", output);
				if(strcmp(line, "01010100000000000000000000001101") == 0)
					instructionsFlag = false;
			}
			else if (strcmp(header, "11") == 0)
            {
				MIPSsimDisassembler.disassemble(line, 2, output, address);
                fprintf(DO, "%s\n",output);
            }
		}
		else {
            int numbers = MIPSsimDisassembler.TransformSignedImmediate(32, line);
            memoryInitial[address] = numbers;
            if (addressDataStart == -1) {
                addressDataStart = address;
            }
            fprintf(DO, "%s\t%d\t%d\n", line, address, numbers);
		}
		address += 4;
	}
    addressDataEnds = address - 4;
    
	fclose(f);
    fclose(DO);

    MIPSSimConsolePipeline consolePipeline("simulation.txt", 256, addressDataStart/4, addressDataEnds/4);
    
    map<int, int>::iterator it;
    for (it = memoryInitial.begin(); it != memoryInitial.end(); it++) {
        consolePipeline.setMemory(it->second, it->first/4);
    }
    consolePipeline.Instructions.clear();
    int arrayPreIssue[4];
    memset(arrayPreIssue, -1, sizeof(int)*4);
    while (!consolePipeline.terminationFlag) {
        consolePipeline.pipelineProcess(MIPSsimDisassembler);
        consolePipeline.pipelineSimulationPrint();
        consolePipeline.countsCycles++;
    }
}