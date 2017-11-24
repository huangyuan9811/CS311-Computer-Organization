/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"
#include "parse.h"


int text_size;
int data_size;
void do_R_instr(short func, unsigned char rs, unsigned char rt,unsigned char rd, unsigned char shamt);
void do_I_instr(short opcode, unsigned char rs, unsigned char rt, short imm);
void do_J_instr(short opcode, uint32_t target);
/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) 
{ 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
	
	/** Implement this function */
	int pc = CURRENT_STATE.PC;
	// calculate NUM_INST once at the beginning
	if (pc == MEM_TEXT_START) {
		NUM_INST = text_size / 4;
	}

// exit if invalid pc 
	if (CURRENT_STATE.PC >= MEM_TEXT_START + text_size) {
		printf("Simulator halted\n\n");
		rdump();
		exit(0);
	}

	instruction * curr_instr = get_inst_info(pc);

	short type = get_instr_type(curr_instr->opcode);
	short func;
	unsigned char rs, rt, rd, shamt;
	short imm;
	uint32_t target;

	if (type ==1) { //	R type
		func = FUNC(curr_instr);
		rs = RS(curr_instr);
		rt = RT(curr_instr);
		rd = RD(curr_instr);
		shamt = SHAMT(curr_instr);
		do_R_instr(func, rs, rt, rd, shamt);
	} else if (type ==2) { // I type
		rs = RS(curr_instr);
		rt = RT(curr_instr);
		imm = IMM(curr_instr);
		do_I_instr(OPCODE(curr_instr), rs, rt, imm);
	} else if (type ==3) { // J type
		target = TARGET(curr_instr);
		do_J_instr(OPCODE(curr_instr), target);
	}
	
	
}

void do_R_instr(short func, unsigned char rs, unsigned char rt,unsigned char rd, unsigned char shamt) {
	CPU_State * cs = &CURRENT_STATE;

	if (func == 8) { // jr instruction
		uint32_t jumpto = CURRENT_STATE.REGS[rs];
		CURRENT_STATE.PC = jumpto;
	} else{
		switch (func) {
			case 0x20: //add, how about overflow?
			{
				cs->REGS[rd] = cs->REGS[rs] + cs->REGS[rt];
				break;
			}
			case 0x21: // addu
			{ 
				cs->REGS[rd] = cs->REGS[rs] + cs->REGS[rt];
				break;
			}
			case 0x24: //and
			{
				cs->REGS[rd] = cs->REGS[rs] & cs->REGS[rt];
				break;
			}
			case 0x27: // nor
			{
				cs->REGS[rd] = ~(cs->REGS[rs] | cs->REGS[rt]);
				break;
			}
			case 0x25: // or
			{
				cs->REGS[rd] = (cs->REGS[rs] | cs->REGS[rt]);
				break;
			}
			case 0x2a: // slt  SIGNED!
			{
				cs->REGS[rd] = (cs->REGS[rs] < cs->REGS[rt]) ? 1 : 0;
				break;
			}
			case 0x2b: // sltu  UNSIGNED!
			{
				cs->REGS[rd] = (cs->REGS[rs] < cs->REGS[rt]) ? 1 : 0;
				break;
			}
			case 0x00: // sll 
			{
				cs->REGS[rd] = cs->REGS[rt] << shamt;
				break;
			}
			case 0x02: // srl , register values are already unsigned so its logical
			{
				cs->REGS[rd] = cs->REGS[rt] >> shamt;
				break;
			}
			case 0x22: // sub
			{
				cs->REGS[rd] = cs->REGS[rs] - cs->REGS[rt];
				break;
			}
			case 0x23: // subu
			{
				cs->REGS[rd] = cs->REGS[rs] - cs->REGS[rt];
				break;
			}

		}
		CURRENT_STATE.PC += 0x4;
	}
}

// do i need to check for invalid pointers?

void do_I_instr(short opcode, unsigned char rs, unsigned char rt, short imm) {

	CPU_State * cs = &CURRENT_STATE;
	if (opcode == 0x4 || opcode == 0x5) { // branch instructions
		if (opcode == 0x4) { // beq
			int signExtImm = imm;
			signExtImm = (imm << 16) >> 16;
			uint32_t branch_addr = signExtImm << 2;
			if (cs->REGS[rs] == cs->REGS[rt]) {
				CURRENT_STATE.PC = CURRENT_STATE.PC + 0x4 + branch_addr;
			} else {
				CURRENT_STATE.PC += 0x4;
			}
		}
		else if (opcode == 0x5) { // bne
			int signExtImm = imm;
			signExtImm = (imm << 16) >> 16;
			uint32_t branch_addr = signExtImm << 2;
			if (cs->REGS[rs] != cs->REGS[rt]) {
				CURRENT_STATE.PC = CURRENT_STATE.PC + 0x4 + branch_addr;
			} else {
				CURRENT_STATE.PC += 0x4;
			}
		}
	} else {
		
		switch(opcode) {
			case 0x8: // addi
			{
				//sign extend imm
				int signExtImm = imm;
				signExtImm = (imm <<16) >> 16;
				cs->REGS[rt] = cs->REGS[rs] + signExtImm;
				break;
			}
			case 0x9: // addiu
			{
				uint32_t extImm = imm;
				extImm = (imm << 16) >> 16;
				cs->REGS[rt] = cs->REGS[rs] + extImm;
				break;
			}
			case 0xc: //andi
			{
				uint32_t zeroExtImm = imm & 0xffff;
				cs->REGS[rt] = cs->REGS[rs] & zeroExtImm;
				break;
			}
			case 0x24: //load byte unsigned
			{
				int signExtImm = imm;
				signExtImm = (imm <<16) >> 16;
				uint32_t addr = cs->REGS[rs] + signExtImm;
				uint32_t mem_value = mem_read_32(addr);
				cs->REGS[rt] = 0xff & mem_value;
				break;
			}
			case 0x25: // load halfword unsigned
			{
				int signExtImm = imm;
				signExtImm = (imm <<16) >> 16;
				uint32_t addr = cs->REGS[rs] + signExtImm;
				uint32_t mem_value = mem_read_32(addr);
				cs->REGS[rt] = 0xffff & mem_value;
				break;
			}
			case 0xf: // lui: load upper imm
			{
				uint32_t upperImm = (imm << 16);
				cs->REGS[rt] = upperImm;
				break;
			}
			case 0x23: // load word
			{
				int signExtImm = imm;
				signExtImm = (imm <<16) >> 16;
				uint32_t addr = cs->REGS[rs] + signExtImm;
				cs->REGS[rt] = mem_read_32(addr);
				break;
			}
			case 0xd: //ori
			{
				unsigned zeroExtImm = imm & 0xffff;
				cs->REGS[rt] = cs->REGS[rs] | zeroExtImm;
				break;
			}
			case 0xa: //set less than imm
			{
				int signExtImm = imm;
				signExtImm = (imm <<16) >> 16;
				cs->REGS[rt] = (cs->REGS[rs] < signExtImm) ? 1 : 0;
				break;
			}
			case 0xb : // set less than imm unsigned
			{
				uint32_t extImm = imm;
				extImm = (imm <<16) >> 16;
				cs->REGS[rt] = (cs->REGS[rs] < extImm) ? 1 : 0;
				break;
			}
			case 0x2b: //store word
			{
				int signExtImm = imm;
				signExtImm = (imm <<16) >> 16;
				uint32_t addr = cs->REGS[rs] + signExtImm;
				mem_write_32(addr, cs->REGS[rt]);
				break;
			}
		}		
		CURRENT_STATE.PC +=0x4;
	}
}

void do_J_instr(short opcode, uint32_t target) {
	CPU_State * cs = &CURRENT_STATE;
	uint32_t pc_4bits = (CURRENT_STATE.PC+0x4) & 0xf0000000;
	uint32_t jump_addr = pc_4bits  | ((target << 2) & 0x0fffffff);

	switch (opcode) {
		case 0x2: // j
		{
			CURRENT_STATE.PC = jump_addr;
			break;
		}
		case 0x3: //jal
		{
			cs->REGS[31] = CURRENT_STATE.PC + 0x8; // PC+8
			CURRENT_STATE.PC = jump_addr;
			break;
		}
	}
}
