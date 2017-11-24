/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"
#include "run.h"

int text_size;
int data_size;


instruction parsing_instr(const char *buffer, const int index)
{
    instruction instr;
	/** Implement this function */

	if (index > text_size) {
		printf("not within the text section\n");
	} else {
		
		// first look at the first 6 bits 
		char opcode[6];
		strncpy(opcode, buffer, 6);
		//(&instr)->opcode = (short) fromBinary(opcode); 
		SET_OPCODE(&instr, fromBinary(opcode));
		
		short op = fromBinary(opcode);
		short type = get_instr_type(op);


		if (type == 3) {
			char target[26];
			strncpy(target, buffer+6, 26);
			(&instr)->r_t.target = (uint32_t) fromBinary(target);
			//SET_TARGET(&instr, (uint32_t) fromBinary(target));
		} 
		if (type == 2 || type == 1) { // fill rs and rt registers
			char rs[5];
			char rt[5];
			strncpy(rs, buffer+6, 5);
			strncpy(rt, buffer+11, 5);
			//(&instr)->r_t.r_i.rs = (unsigned char) fromBinary(rs);
			//(&instr)->r_t.r_i.rt = (unsigned char) fromBinary(rt);
			SET_RS(&instr, fromBinary(rs));
			SET_RT(&instr, fromBinary(rt));
		}

		if (type ==2) { // fill immediate
			char imm[16];
			strncpy(imm, buffer+16, 16);
			//(&instr)->r_t.r_i.r_i.imm = (short) fromBinary(imm);
			SET_IMM(&instr, fromBinary(imm));
		}

		if (type == 1) {
			// fill func, rd, shamt
			char func[6];
			char rd[5];
			char shamt[5];
			strncpy(func, buffer+26, 6);
			strncpy(rd, buffer+16,5);
			strncpy(shamt, buffer+21, 5);
			//(&instr)->func_code = (short) fromBinary(func);
			//(&instr)->r_t.r_i.r_i.r.rd = (unsigned char) fromBinary(rd);
			//(&instr)->r_t.r_i.r_i.r.shamt = (unsigned char) fromBinary(shamt);
			SET_FUNC(&instr, fromBinary(func));
			SET_RD(&instr, fromBinary(rd));
			SET_SHAMT(&instr, fromBinary(shamt));
		}
	}
    return instr;
}

void parsing_data(const char *buffer, const int index)
{
	/** Implement this function */

	uint32_t addr = (MEM_DATA_START) + index;
	mem_write_32(addr, (uint32_t) fromBinary((char *) buffer));

}

void print_parse_result()
{
    int i;
    printf("Instruction Information\n");

    for(i = 0; i < text_size/4; i++)
    {
	printf("INST_INFO[%d].value : %x\n",i, INST_INFO[i].value);
	printf("INST_INFO[%d].opcode : %d\n",i, INST_INFO[i].opcode);

	switch(INST_INFO[i].opcode)
	{
	    //Type I
	    case 0x9:		//(0x001001)ADDIU
	    case 0xc:		//(0x001100)ANDI
	    case 0xf:		//(0x001111)LUI	
	    case 0xd:		//(0x001101)ORI
	    case 0xb:		//(0x001011)SLTIU
	    case 0x23:		//(0x100011)LW	
	    case 0x2b:		//(0x101011)SW
	    case 0x4:		//(0x000100)BEQ
	    case 0x5:		//(0x000101)BNE
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].imm : %d\n",i, INST_INFO[i].r_t.r_i.r_i.imm);
		break;

    	    //TYPE R
	    case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
		printf("INST_INFO[%d].func_code : %d\n",i, INST_INFO[i].func_code);
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].rd : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.rd);
		printf("INST_INFO[%d].shamt : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
		break;

    	    //TYPE J
	    case 0x2:		//(0x000010)J
	    case 0x3:		//(0x000011)JAL
		printf("INST_INFO[%d].target : %d\n",i, INST_INFO[i].r_t.target);
		break;

	    default:
		printf("Not available instruction\n");
		assert(0);
	}
    }

    printf("Memory Dump - Text Segment\n");
    for(i = 0; i < text_size; i+=4)
	printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
    for(i = 0; i < data_size; i+=4)
	printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
    printf("Current PC: %x\n", CURRENT_STATE.PC);
}


short get_instr_type(short op) {
	if (op == 0) return 1; // R-type
	else if (op == 2 || op == 3) return 3; // J-type
	else 
		return 2; // I-type
}
