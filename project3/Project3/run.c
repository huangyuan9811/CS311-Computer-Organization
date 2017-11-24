/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdbool.h>

#include "util.h"
#include "run.h"

int text_size;
short get_instr_type(short op);
void parse_control_signals(instruction * instr);
bool alu_out_zero = 0;
int ID_EX_num_stall = 0; // for hazard detection. load followed by alu instructions
int IF_ID_num_stall = 0; // for jump instructions and branching
int jump_IF_ID_num_stall = 0;
/* Next_PC = set it to the PC of the next instruction to fetch
   1. PC + 4
   2. Branch address
   3. Jump address
 */
uint32_t Next_PC = 0;
uint32_t last_PC = 0;
bool jumpInstr = 0;
bool jumpandlink = 0;
bool jumpandreturn = 0;
bool branchPredictionFailed = 0;

void flush_IF_ID();
void flush_ID_EX();
void flush_EX_MEM();

int fill_pipeline = 0;

int cycle_count = 0;
bool finished = 0;
bool next_finished = 0;
bool fix_PC = 0; // To set PC value when instructions are all finished
bool hazard_detected = 0;

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) { 

	/* check if the current instruction is out of instruction bound */
	if ((pc) >= text_size + MEM_TEXT_START) {
		finished = 1;
		fix_PC = 1;
		last_PC = pc;
		return NULL;
	}
	if ((pc + 4) >= text_size + MEM_TEXT_START) { 
		next_finished = 1;
	}
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
	cycle_count ++;
	/** Your implementation here */

	/* take care of all the flushing and branching */
	// update current pc. to find out which instruction to fetch in this cycle.
	// 3 different types : jump address, branch address, PC+4

	/* initial PC */ 
	if (cycle_count-1 == 0) {
		Next_PC = CURRENT_STATE.PC;
	}

	if (finished) {
		if (fix_PC) {
			CURRENT_STATE.PC = Next_PC;
			fix_PC = 0;
		}
		/* Set Next_PC to 0 to prevent more instructions from being fetched */
		Next_PC= 0;
	} else {
		CURRENT_STATE.PC = Next_PC + 4;
	}

	/* Handle IF/ID stalls for jump instruction */
	if (jump_IF_ID_num_stall > 0) {
		flush_IF_ID();
	}
	if (jump_IF_ID_num_stall > 0) jump_IF_ID_num_stall--;

	/* dump out the register */
	if (cycle_count >1) {
		if (CURRENT_STATE.PIPE[IF_STAGE] ==0 && CURRENT_STATE.PIPE[ID_STAGE] ==0 && CURRENT_STATE.PIPE[EX_STAGE] ==0 && CURRENT_STATE.PIPE[MEM_STAGE] == 0 && CURRENT_STATE.PIPE[WB_STAGE] ) {
		rdump();
		}
	}

	IF_Stage();
	WB_Stage();
	ID_Stage();
	EX_Stage();
	MEM_Stage();

	if (ID_EX_num_stall > 0) {
		flush_ID_EX();
	}

	if ((IF_ID_num_stall > 0 && ID_EX_num_stall ==0) || finished ) {
		if (finished) {
		CURRENT_STATE.PIPE[IF_STAGE] = 0;
		CURRENT_STATE.PC = last_PC;
		}
		flush_IF_ID();
		
	} 

	/* if data hazard occurs, don't update the registers */
	if (ID_EX_num_stall == 0) {
		CURRENT_STATE.past_IF_ID_pipeline = CURRENT_STATE.new_IF_ID_pipeline;
		CURRENT_STATE.past_ID_EX_pipeline = CURRENT_STATE.new_ID_EX_pipeline;
	}
	CURRENT_STATE.past_EX_MEM_pipeline = CURRENT_STATE.new_EX_MEM_pipeline;
	CURRENT_STATE.past_MEM_WB_pipeline = CURRENT_STATE.new_MEM_WB_pipeline;

	if (IF_ID_num_stall > 0) IF_ID_num_stall --;
	if (ID_EX_num_stall > 0) ID_EX_num_stall --;

	/* if hazard is detected, current PC is not updated */
	if (hazard_detected) {
		CURRENT_STATE.PC -= 4;
		hazard_detected = 0;
	}

	/* If all pipeline is empty, exit(0) */
	if (cycle_count >1) {
		if (CURRENT_STATE.PIPE[IF_STAGE] ==0 && CURRENT_STATE.PIPE[ID_STAGE] ==0 && CURRENT_STATE.PIPE[EX_STAGE] ==0 && CURRENT_STATE.PIPE[MEM_STAGE] == 0 && CURRENT_STATE.PIPE[WB_STAGE] ==0 ) {
		printf("Simulator halted after %d cycles\n\n", cycle_count-1);
			exit(0);
		}
	}
}

void IF_Stage() {

	// fetch instruction
	instruction * instr = get_inst_info(Next_PC);
	if (instr == NULL) return;

	CURRENT_STATE.PIPE[IF_STAGE] = Next_PC;

	/* dont know why it doesnt empty out by itself.. */
	if (branchPredictionFailed) {
		CURRENT_STATE.PIPE[IF_STAGE] = 0;
		branchPredictionFailed = 0;
	}
	// save this PC
	CURRENT_STATE.new_IF_ID_pipeline.IF_ID_NPC = Next_PC;

	if (finished) return;
	// save it in the IF/ID register
	CURRENT_STATE.new_IF_ID_pipeline.IF_ID_INST = instr;

	// default condition
	Next_PC +=4;
}

void ID_Stage() {
	CURRENT_STATE.PIPE[ID_STAGE] = CURRENT_STATE.past_IF_ID_pipeline.IF_ID_NPC ;

	CPU_State * cs = &CURRENT_STATE;
	// pass PC+4 to ID/EX register
	cs->new_ID_EX_pipeline.ID_EX_NPC = cs->past_IF_ID_pipeline.IF_ID_NPC;
	cs->new_ID_EX_pipeline.ID_EX_INST = cs->past_IF_ID_pipeline.IF_ID_INST;

	/* read the old instruction */
	instruction * instr = cs->past_IF_ID_pipeline.IF_ID_INST;
	if (instr == NULL) return;

	/* parse and save to new ID/EX registers */
	parse_control_signals(instr);
	/* save/pass register1, register2, sign extended imm, pc+4 */
	cs->new_ID_EX_pipeline.ID_EX_REG1 = cs->REGS[RS(instr)];
	cs->new_ID_EX_pipeline.ID_EX_REG2 = cs->REGS[RT(instr)];
	cs->new_ID_EX_pipeline.ID_EX_RS = RS(instr); // register number
	cs->new_ID_EX_pipeline.ID_EX_RT = RT(instr); // register number 
	cs->new_ID_EX_pipeline.ID_EX_RD = RD(instr);
	/* zero extend vs sign extend */
	if (OPCODE(instr) == 0xc || OPCODE(instr) == 0xd) cs->new_ID_EX_pipeline.ID_EX_IMM = (IMM(instr));
	else 
		cs->new_ID_EX_pipeline.ID_EX_IMM = SIGN_EX(IMM(instr));

	// if R-type instruction, dest == RD, else RT
	short instr_type = get_instr_type(OPCODE(instr));
	if (instr_type == 1){ 
		cs->new_ID_EX_pipeline.ID_EX_DEST = RD(instr);
	}
	else if (instr_type == 2){ 
		cs->new_ID_EX_pipeline.ID_EX_DEST = RT(instr);
	}

	/* hazard detection 
	   ex. add after load
	   - decrease Next_PC that was incremented in IF stage as default
	 */
	if (cs->past_ID_EX_pipeline.MEM_CONTROL.MEM_READ) {
		if (cs->new_ID_EX_pipeline.ID_EX_RS  == cs->past_ID_EX_pipeline.ID_EX_RT) {
			ID_EX_num_stall = 1;
			Next_PC -=4;
			hazard_detected = 1;
		} else if (cs->new_ID_EX_pipeline.ID_EX_RT == cs->past_ID_EX_pipeline.ID_EX_RT) {
			ID_EX_num_stall = 1;
			Next_PC -=4;
			hazard_detected = 1;
		}
	}

}

void EX_Stage() {
	CURRENT_STATE.PIPE[EX_STAGE] = CURRENT_STATE.past_ID_EX_pipeline.ID_EX_NPC;
	CPU_State * cs = &CURRENT_STATE;
	/* pass necessary registers and control signals onto the next EX/MEM registers */
	cs->new_EX_MEM_pipeline.EX_MEM_NPC = cs->past_ID_EX_pipeline.ID_EX_NPC;
	cs->new_EX_MEM_pipeline.EX_MEM_RD = cs->past_ID_EX_pipeline.ID_EX_RD;

	cs->new_EX_MEM_pipeline.MEM_CONTROL.MEM_READ = cs->past_ID_EX_pipeline.MEM_CONTROL.MEM_READ;
	cs->new_EX_MEM_pipeline.MEM_CONTROL.MEM_WRITE = cs->past_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE;
	cs->new_EX_MEM_pipeline.MEM_CONTROL.PC_SRC = cs->past_ID_EX_pipeline.MEM_CONTROL.PC_SRC;
	cs->new_EX_MEM_pipeline.WB_CONTROL.REG_WRITE = cs->past_ID_EX_pipeline.WB_CONTROL.REG_WRITE;
	cs->new_EX_MEM_pipeline.WB_CONTROL.MEM_TO_REG = cs->past_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG;

	/* check for data forwarding */
	int forwardA = 0;
	int forwardB = 0;

	// EX
	if (cs->past_EX_MEM_pipeline.WB_CONTROL.REG_WRITE) {
		if (cs->past_EX_MEM_pipeline.EX_MEM_DEST != 0) {
			if (cs->past_EX_MEM_pipeline.EX_MEM_DEST == cs->past_ID_EX_pipeline.ID_EX_RS) 			{
				forwardA = 2;
			}
			if (cs->past_EX_MEM_pipeline.EX_MEM_DEST == cs->past_ID_EX_pipeline.ID_EX_RT) 			{
				forwardB = 2;
			}

		}
	}

	// MEM
	if (cs->past_MEM_WB_pipeline.WB_CONTROL.REG_WRITE) {
		if (cs->past_MEM_WB_pipeline.MEM_WB_DEST != 0 ){
			if (cs->past_EX_MEM_pipeline.EX_MEM_DEST != cs->past_ID_EX_pipeline.ID_EX_RS) 			{	

				if (cs->past_MEM_WB_pipeline.MEM_WB_DEST == cs->past_ID_EX_pipeline.ID_EX_RS) 			{
					forwardA = 1;
				}
			}
			if (cs->past_EX_MEM_pipeline.EX_MEM_DEST != cs->past_ID_EX_pipeline.ID_EX_RT) 			{
				if (cs->past_MEM_WB_pipeline.MEM_WB_DEST == cs->past_ID_EX_pipeline.ID_EX_RT) 			{
					forwardB = 1;
				}
			}
		}
	}

		/* MUX for reg dest , rt or rd */
		if (cs->past_ID_EX_pipeline.EX_CONTROL.REG_DST) { // asserted == rd
			cs->new_EX_MEM_pipeline.EX_MEM_DEST = cs->past_ID_EX_pipeline.ID_EX_DEST;
		} else {
			cs->new_EX_MEM_pipeline.EX_MEM_DEST = cs->past_ID_EX_pipeline.ID_EX_RT;
		}

		/* MUX with forwarding unit for ALU input */
		uint32_t ALU_IN_1;
		uint32_t ALU_IN_2;
		if (forwardA == 0) { // from ID/EX REG1
			ALU_IN_1 = cs->past_ID_EX_pipeline.ID_EX_REG1;
		} else if (forwardA== 1) { // from after WB
			if (cs->past_MEM_WB_pipeline.WB_CONTROL.MEM_TO_REG) {
				ALU_IN_1 = cs->past_MEM_WB_pipeline.MEM_WB_MEM_OUT;
			} else {
				ALU_IN_1 = cs->past_MEM_WB_pipeline.MEM_WB_ALU_OUT;
			}
		} else if (forwardA == 2) { // from EX/MEM
			ALU_IN_1 = cs->past_EX_MEM_pipeline.EX_MEM_ALU_OUT;
		}

		if (forwardB == 0) { // from ID/EX REG2
			ALU_IN_2 = cs->past_ID_EX_pipeline.ID_EX_REG2;
		} else if (forwardB == 1) { // from after WB
			if (cs->past_MEM_WB_pipeline.WB_CONTROL.MEM_TO_REG) {
				ALU_IN_2 = cs->past_MEM_WB_pipeline.MEM_WB_MEM_OUT;
			} else {
				ALU_IN_2 = cs->past_MEM_WB_pipeline.MEM_WB_ALU_OUT;
			}
		} else if (forwardB == 2) { // from EX/MEM
			ALU_IN_2 = cs->past_EX_MEM_pipeline.EX_MEM_ALU_OUT;
		}
		// save it in EX/MEM register
		cs->new_EX_MEM_pipeline.EX_MEM_ALU_IN_2 = ALU_IN_2;
		// from sign extended imm or from forwardB output
		if (cs->past_ID_EX_pipeline.EX_CONTROL.ALU_SRC) {
			ALU_IN_2 = cs->past_ID_EX_pipeline.ID_EX_IMM;
		}
		/* 6 ALU operations pg.247 + alpha */
		short control = cs->past_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL;
		uint32_t alu_out;
		cs->new_EX_MEM_pipeline.EX_MEM_ZERO_OUT = 0;
		if (control == 0) { // AND
			alu_out = ALU_IN_1 & ALU_IN_2;
		} else if (control == 2) { // ADD
			alu_out = ALU_IN_1 + ALU_IN_2;
		} else if (control == 1) { // OR
			alu_out = ALU_IN_1 | ALU_IN_2;
		} else if (control == 6) { // SUB
			alu_out = ALU_IN_1 - ALU_IN_2;
			if (alu_out == 0) {
				cs->new_EX_MEM_pipeline.EX_MEM_ZERO_OUT = 1;
			}
		} else if (control == 8) { // for BNE
			alu_out = ALU_IN_1 - ALU_IN_2;
			if (alu_out != 0){
				cs->new_EX_MEM_pipeline.EX_MEM_ZERO_OUT = 1;
			}

		} else if (control == 7) { // SLT
			alu_out = ALU_IN_1 < ALU_IN_2;
		} else if (control == 12) { // NOR
			alu_out = ~(ALU_IN_1 | ALU_IN_2);
		} else if (control == 3 || control == 4) { // sll or srl
			uint32_t shamt = SHAMT(cs->past_ID_EX_pipeline.ID_EX_INST);
			if (control == 3) {
				alu_out = (ALU_IN_2 << shamt);
			} else if (control == 4) {
				alu_out = (ALU_IN_2 >> shamt);
			}
		} else if (control == 9 ) { // lui
			alu_out = (ALU_IN_2 << 16);
		}
		cs->new_EX_MEM_pipeline.EX_MEM_ALU_OUT = alu_out;


		/* branch address calculation and check if branch is taken or not */
		cs->new_EX_MEM_pipeline.EX_MEM_BR_TARGET = ((cs->past_ID_EX_pipeline.ID_EX_IMM) <<2) + cs->past_ID_EX_pipeline.ID_EX_NPC + 4;
		if (cs->new_EX_MEM_pipeline.EX_MEM_ZERO_OUT && cs->past_ID_EX_pipeline.EX_CONTROL.BRANCH) {
			branchPredictionFailed = 1;
			IF_ID_num_stall = 3;
			Next_PC = cs->new_EX_MEM_pipeline.EX_MEM_BR_TARGET;
		}

}

void MEM_Stage() {
	/* check for branch prediction failed or not */
	if (branchPredictionFailed) {
		// flush!
		//	flush_EX_MEM(); // did i update this or not?
		flush_ID_EX();
		flush_IF_ID();
		//CURRENT_STATE.PC = Next_PC; 
		IF_ID_num_stall--;
		//branchPredictionFailed = 0;
		/* undo the default PC+4 */
		Next_PC -=4;
	}


	CURRENT_STATE.PIPE[MEM_STAGE] = CURRENT_STATE.past_EX_MEM_pipeline.EX_MEM_NPC;

	CPU_State * cs = &CURRENT_STATE;
	cs->new_MEM_WB_pipeline.MEM_WB_NPC = cs->past_EX_MEM_pipeline.EX_MEM_NPC;
	// pass on control signals to WB
	cs->new_MEM_WB_pipeline.WB_CONTROL.REG_WRITE = cs->past_EX_MEM_pipeline.WB_CONTROL.REG_WRITE;
	cs->new_MEM_WB_pipeline.WB_CONTROL.MEM_TO_REG = cs->past_EX_MEM_pipeline.WB_CONTROL.MEM_TO_REG;
	// pc +4 
	cs->new_MEM_WB_pipeline.MEM_WB_NPC = cs->past_EX_MEM_pipeline.EX_MEM_NPC; 
	cs->new_MEM_WB_pipeline.MEM_WB_RD = cs->past_EX_MEM_pipeline.EX_MEM_RD;
	cs->new_MEM_WB_pipeline.MEM_WB_DEST = cs->past_EX_MEM_pipeline.EX_MEM_DEST;

	// operate on data memory
	if (cs->past_EX_MEM_pipeline.MEM_CONTROL.MEM_READ) {
		// lw instruction
		cs->new_MEM_WB_pipeline.MEM_WB_MEM_OUT = mem_read_32(cs->past_EX_MEM_pipeline.EX_MEM_ALU_OUT);
	} else { 
		cs->new_MEM_WB_pipeline.MEM_WB_ALU_OUT = cs->past_EX_MEM_pipeline.EX_MEM_ALU_OUT; // for r-type instruction, this will be written back to destination register in WB stage
	}
	if (cs->past_EX_MEM_pipeline.MEM_CONTROL.MEM_WRITE){
		// sw instruction
		mem_write_32(cs->past_EX_MEM_pipeline.EX_MEM_ALU_OUT, cs->past_EX_MEM_pipeline.EX_MEM_ALU_IN_2); // check page 280 and 313
	}

}

void WB_Stage() {
	CURRENT_STATE.PIPE[WB_STAGE] = CURRENT_STATE.past_MEM_WB_pipeline.MEM_WB_NPC;
	CPU_State * cs = &CURRENT_STATE;
	// select between data from alu or from memory
	uint32_t data;
	if(cs->past_MEM_WB_pipeline.WB_CONTROL.MEM_TO_REG){
		data = cs->past_MEM_WB_pipeline.MEM_WB_MEM_OUT;
	} else {
		data = cs->past_MEM_WB_pipeline.MEM_WB_ALU_OUT;
	}

	/* write to destination register... problem here. maybe related to synch issues of doing wb before other instruction */
	if (cs->past_MEM_WB_pipeline.WB_CONTROL.REG_WRITE) {
		cs->REGS[cs->past_MEM_WB_pipeline.MEM_WB_DEST] = data;
	}
	
	/* for stopping the instruction in cs311.c */
	if (CURRENT_STATE.PIPE[WB_STAGE] != 0) { 
		INSTRUCTION_COUNT ++;
	}
}

void parse_control_signals(instruction * instr) {
	CPU_State * cs = &CURRENT_STATE;
	// what kind of control signals?
	if (OPCODE(instr) == 0) {
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 2;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
		switch (FUNC(instr)){
			case 0x20: //add
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 2;
				break;
			case 0x22: //sub
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 6;
				break;
			case 0x24: // and
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 0;
				break;
			case 0x25 : // or
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 1;
				break;
			case 0x2a: // slt
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 7;
				break;
			case 0x8: // jr
				jumpInstr = 1;
				jumpandreturn = 1;
				cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
				break;
			case 0x21 : // addu
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 2;
				break;
			case 0x27 : // nor
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 12;
				break;
			case 0x2b : // sltu
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 7;
				break;
			case 0x0 : // sll
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 3; // made up 
				break;
			case 0x2: // srl
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 4; // made up
				break;
			case 0x23 : // sub
				cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 6;
				break;
			default : 
				break;
		}

		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;

		if (!jumpInstr) {
			cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 1;
		} else {
			cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 0;
		}
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
	} 
	/* I-type */
	else if (OPCODE(instr) == 0x23) { // lw
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 2;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 1;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 1;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 1;
	} else if (OPCODE(instr) == 0x2b) { // sw
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 2;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 1;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
	} else if (OPCODE(instr) == 0x4) { // beq
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 6;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 1;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 1;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
	} else if (OPCODE(instr) == 0x5) { // bne 
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 8;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 1;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 1;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
	} else if (OPCODE(instr) == 0x9) { // addiu
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 2;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 2;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 1;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
	} else if (OPCODE(instr) == 0xc) { //andi
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 2;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 1;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
	} else if (OPCODE(instr) == 0xf) { // lui
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 9;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 1;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
	} else if (OPCODE(instr) == 0xd) { // ori
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 1;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
	} else if (OPCODE(instr) == 0xb) { //sltiu
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 7;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 1;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
	} 
	/* J-type */
	else if (OPCODE(instr) == 0x2) { // j
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
		jumpInstr = 1;
	} else if (OPCODE(instr) == 0x3) { //jal
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 1;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 0;
		cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
		cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
		cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 1;
		cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;
		jumpInstr = 1;
		jumpandlink = 1;
		cs->REGS[31] = cs->past_ID_EX_pipeline.ID_EX_NPC + 0x8 + 0x4; // PC+8
	}

	if (jumpInstr) { 
		/* stall one cycle always */
		jump_IF_ID_num_stall = 1;
		/* calculate jump address */
		uint32_t target = TARGET(instr);
		uint32_t pc_4bits = (cs->past_ID_EX_pipeline.ID_EX_NPC) & 0xf0000000;
		Next_PC = pc_4bits | ((target<<2) & 0x0fffffff) ;
		if (jumpandreturn) {
			Next_PC = cs->REGS[31] ;
		}
		jumpInstr = 0;
		jumpandlink = 0;
		jumpandreturn = 0;
	}
}

void flush_IF_ID(){
	CPU_State * cs = &CURRENT_STATE;
//	cs->PIPE[IF_STAGE] = 0;
	cs->new_IF_ID_pipeline.IF_ID_INST = 0;
	cs->new_IF_ID_pipeline.IF_ID_NPC = 0;
	cs->past_IF_ID_pipeline.IF_ID_INST = 0;
	cs->past_IF_ID_pipeline.IF_ID_NPC = 0;
}

void flush_ID_EX(){
	CPU_State * cs = &CURRENT_STATE;
	cs->new_ID_EX_pipeline.ID_EX_INST = 0;
	cs->new_ID_EX_pipeline.ID_EX_NPC = 0;
	cs->new_ID_EX_pipeline.ID_EX_REG1 = 0;
	cs->new_ID_EX_pipeline.ID_EX_REG2 = 0;
	cs->new_ID_EX_pipeline.ID_EX_IMM = 0;
	cs->new_ID_EX_pipeline.ID_EX_DEST = 0;
	cs->new_ID_EX_pipeline.ID_EX_RS = 0;
	cs->new_ID_EX_pipeline.ID_EX_RT = 0;
	/* ID_EX_EX_CONTROL */
	cs->new_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 0;
	cs->new_ID_EX_pipeline.EX_CONTROL.ALU_OP = 0;
	cs->new_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 0;
	cs->new_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
	cs->new_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
	/* ID_EX_MEM_CONTROL */
	cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
	cs->new_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
	cs->new_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
	/* ID_EX_WB_CONTROL */
	cs->new_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 0;
	cs->new_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;

	cs->past_ID_EX_pipeline.ID_EX_INST = 0;
	cs->past_ID_EX_pipeline.ID_EX_NPC = 0;
	cs->past_ID_EX_pipeline.ID_EX_REG1 = 0;
	cs->past_ID_EX_pipeline.ID_EX_REG2 = 0;
	cs->past_ID_EX_pipeline.ID_EX_IMM = 0;
	cs->past_ID_EX_pipeline.ID_EX_DEST = 0;
	cs->past_ID_EX_pipeline.ID_EX_RS = 0;
	cs->past_ID_EX_pipeline.ID_EX_RT = 0;
	/* ID_EX_EX_CONTROL */
	cs->past_ID_EX_pipeline.EX_CONTROL.ALU_SRC = 0;
	cs->past_ID_EX_pipeline.EX_CONTROL.ALU_OP = 0;
	cs->past_ID_EX_pipeline.EX_CONTROL.ALU_CONTROL = 0;
	cs->past_ID_EX_pipeline.EX_CONTROL.BRANCH = 0;
	cs->past_ID_EX_pipeline.EX_CONTROL.REG_DST = 0;
	/* ID_EX_MEM_CONTROL */
	cs->past_ID_EX_pipeline.MEM_CONTROL.MEM_READ = 0;
	cs->past_ID_EX_pipeline.MEM_CONTROL.MEM_WRITE = 0;
	cs->past_ID_EX_pipeline.MEM_CONTROL.PC_SRC = 0;
	/* ID_EX_WB_CONTROL */
	cs->past_ID_EX_pipeline.WB_CONTROL.REG_WRITE = 0;
	cs->past_ID_EX_pipeline.WB_CONTROL.MEM_TO_REG = 0;

}

void flush_EX_MEM() {
	CPU_State * cs = &CURRENT_STATE;
	cs->new_EX_MEM_pipeline.EX_MEM_NPC = 0;
	cs->new_EX_MEM_pipeline.EX_MEM_ALU_OUT = 0;
	cs->new_EX_MEM_pipeline.EX_MEM_BR_TARGET = 0;
	cs->new_EX_MEM_pipeline.EX_MEM_DEST = 0;
	cs->new_EX_MEM_pipeline.EX_MEM_ALU_IN_2 = 0;
	/* EX_MEM_MEM_CONTROL */
	cs->new_EX_MEM_pipeline.MEM_CONTROL.MEM_READ = 0;
	cs->new_EX_MEM_pipeline.MEM_CONTROL.MEM_WRITE = 0;
	cs->new_EX_MEM_pipeline.MEM_CONTROL.PC_SRC = 0;
	/* EX_MEM_WB_CONTROL */
	cs->new_EX_MEM_pipeline.WB_CONTROL.REG_WRITE = 0;
	cs->new_EX_MEM_pipeline.WB_CONTROL.MEM_TO_REG = 0;

	cs->past_EX_MEM_pipeline.EX_MEM_NPC = 0;
	cs->past_EX_MEM_pipeline.EX_MEM_ALU_OUT = 0;
	cs->past_EX_MEM_pipeline.EX_MEM_BR_TARGET = 0;
	cs->past_EX_MEM_pipeline.EX_MEM_DEST = 0;
	cs->past_EX_MEM_pipeline.EX_MEM_ALU_IN_2 = 0;
	/* EX_MEM_MEM_CONTROL */
	cs->past_EX_MEM_pipeline.MEM_CONTROL.MEM_READ = 0;
	cs->past_EX_MEM_pipeline.MEM_CONTROL.MEM_WRITE = 0;
	cs->past_EX_MEM_pipeline.MEM_CONTROL.PC_SRC = 0;
	/* EX_MEM_WB_CONTROL */
	cs->past_EX_MEM_pipeline.WB_CONTROL.REG_WRITE = 0;
	cs->past_EX_MEM_pipeline.WB_CONTROL.MEM_TO_REG = 0;

}

short get_instr_type(short op) {
	if (op==0) return 1; // R type
	else if (op==2 || op==3) return 3; // J type
	else 
		return 2; // I type
}




