#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "parseline.h"
#include "table.h"

/*
   R-type 
   opcode  rs  rt  rd  shamt  funct
   6     5   5   5    5       6

   I-type
   opcode  rs  rt  imm
   6     5   5   16

   J-type
   opcode  add
   6     26
 */


int write_to_file(FILE * writefile, char * one_line_instr, int totallen);

int main(int argc, char* argv[]){

	FILE *fp;
	FILE *fp2;
	char *line = NULL;
	size_t len =0;
	ssize_t read;
	char *token;
	int is_data_section = 0;
	int is_word = 0;
	int is_text_section = 0;
	int heapaddr = 0x10000000;
	int pc = 0x100000;
	int type = -1;
	int instr_type;
	int pos; // position within the 32 bit of instruction
	int num_param; // number of expected parameters of each instr type
	int sign_extend = 0; // sign extend or zero extend?
	struct global_variable * head = NULL;
	int data_section_size = 0;
	int text_section_size = 0;
	int num_instructions = 0;
	int is_array = 1;
	int num_array_elem = 0;
	int is_la = 0;
	int lower_16_zeros= 1;
	int is_memory_access = 0;
	char mem_access_register[2] = "" ;
	int save_mem_value = 0;
	int mem_value = 0;
	int two_instr_addr;
	int can_write = 0;
	int num_global_var = 0;
	int totallen = 32;

	// input command ex : ./runfile example1.s
	if (argc !=2){
		printf("Not enough input\n");
	}else {
		/******************** first pass **************************/ 
		fp = fopen(argv[1], "r");
		if (fp==0) {
			printf("Unable to open file\n");
		} else {
			while ((read = getline(&line, &len, fp )) != -1) {
				char * one_line_instr = (char *) malloc(32* sizeof(char));
				token = strtok(line, " \t");
				while (token != NULL) {
					type = token_type(token, one_line_instr, &sign_extend);
					if (type == 10) is_data_section = 1; 
					else if (type ==11 ) { // .word
						if (is_data_section) {
							data_section_size +=32;
							heapaddr += 0x4;
						}
						if (!is_array) {
							// this line of code has name : .word 100
						} else if (is_array) {
							// this line of code only has .word 100
							printf("no label....\n");
							num_array_elem +=1;

						}
					}
					else if (type == 12) {
						is_data_section = 0;
						is_text_section = 1;
					} 
					else if (type == 0) {
						instr_type = get_instr_binary(token, one_line_instr, &sign_extend);
						if (instr_type == 4) {
							// la instruction
							is_la = 1;
						} 
						else if (instr_type == 2 || instr_type == 5) {
							if (strcmp(token, "lw") == 0 ) is_memory_access = 1;
							else if (strcmp(token, "sw") ==0) is_memory_access =1;
						}
						text_section_size += 32;
						pc += 0x1;

					}
					else if (type == 2) { // some sort of "label" name
						if (is_data_section) {
							is_array = 0; 
							num_array_elem = 1;
							token[strlen(token) -1] = 0;
							head = insert_variable(token, head, heapaddr, 1);
							num_global_var ++;
						} else if (is_text_section) { // save pc of the label
							token[strlen(token) -1] = 0;
							head = insert_variable(token, head, pc, 0);
						}
					} 
					else if (type == 3) { // variable or number
						if (is_data_section) {
							//	if (!is_array) {
							set_value_heap(token, num_array_elem, 32, head, 0);
							//		}
						}
						else if (is_text_section) {
							if (is_la){ 
								// if the token(which is a name)'s lower 16 bits are 0s then pass but if not add 32 bits 
								int val;
								val = get_addr(token, head);
								lower_16_zeros = check_lower_16bit(val);
								if (!lower_16_zeros) {
									printf("lower is not zero! so add more instr %d\n", val);
									two_instr_addr = pc - 0x1; 
									text_section_size += 32;
									pc +=0x1;
								}
							} else if (is_memory_access) {
								// need to save which register!
								if (strncmp(token +1, "(", 1) == 0) {
									strncpy(mem_access_register, token+2, 2); 
								} else if (strncmp(token+2, "(", 1) ==0) {
									strncpy(mem_access_register, token+3, 2);
								}
							}
						}
					}
					token = strtok(NULL, " \t"); // get next token
				}
				is_la = 0;
				is_array = 1;
				is_memory_access= 0;
				free(one_line_instr);
			}
		}
		printf("length of data section %d\n", data_section_size);
		printf("length of text section %d\n", text_section_size);
		fclose(fp);

		/******************** second pass **************************/ 
		pc = 0x100000;
		// argv[1] is the assembly file
		fp = fopen(argv[1], "r");
		// remove last char "s" and replace with "o"
		char * lasttok  =strrchr(argv[1], '/');
		if (lasttok != NULL) {
			lasttok[strlen(lasttok)-1] = 'o';
			fp2 = fopen(lasttok+1, "a");
		} else {
			argv[1][strlen(argv[1]) -1] = 'o';
			fp2 = fopen(argv[1], "a");
		}

		if (fp ==0 ){
			printf("Unable to open file\n");
		}
		else {
			// while not end-of-file, read each line
			char * textsizebin = (char *)malloc(sizeof(char) * 32);
			char * datasizebin = (char *)malloc(sizeof(char) * 32);
			dec_to_binary(text_section_size/8, textsizebin, 32);
			dec_to_binary(data_section_size/8, datasizebin, 32);
			write_to_file(fp2, textsizebin,32);
			write_to_file(fp2, datasizebin,32);
			free(textsizebin);
			free(datasizebin);
			while ((read = getline(&line, &len, fp )) != -1) {
				char * one_line_instr = (char *) malloc(32* sizeof(char));

				// parse the line by space(s) to get each word
				token = strtok(line, " \t");
				while (token != NULL) {
					printf("%s\n", token);

					// get binary of each component depending on its type
					type = token_type(token, one_line_instr, &sign_extend);

					// FIRST, check if token indicates section 
					if (type == 10)  { 
						is_data_section = 1;
						can_write = 0;
					}
					else if (type == 12) {
						is_data_section = 0;
						is_text_section = 1;
					}

					else if (type == 0) {
						// it is an instruction
						can_write = 1;
						instr_type = get_instr_binary(token, one_line_instr, &sign_extend);
						if (instr_type == 1) {
							pos = 16; 
							num_param = 3; 
							if (strcmp(token, "jr") ==0 ) {
								pos = 6;
								strncpy(one_line_instr+11, "000000000000000",15); //fill rt
								num_param = 1;
							}		
						}
						else if (instr_type == 2) {
							pos =11;
							num_param = 2;
							if (strcmp(token, "lw") == 0 ) is_memory_access = 1;
							else if (strcmp(token, "sw") ==0) is_memory_access =1;
							if (strncmp(token, "lui",3) ==0 ) {
								num_param = 1;
								strncpy(one_line_instr+6, "00000", 5);
							}
						} else if (instr_type == 5) {
							pos = 6;
							num_param = 2;
						}
						else if (instr_type == 3) {
							pos =6;
							num_param = 1;
						}
						else if (instr_type ==4 ) { // "la" instruction 
							if (two_instr_addr == pc) {
								// if this is the expanded 2 instruction;
								one_line_instr = (char *) realloc(one_line_instr, 64);
								strncpy(one_line_instr, "001111", 6);
								strncpy(one_line_instr+32, "001101", 6);
								pc+=0x1;
								totallen=64;
							} else {
								// just do lui	
								strncpy(one_line_instr, "001111", 6);
							}
							// lui doesnt use rs, so fill it with 0s
							strncpy(one_line_instr+6, "00000", 5);
							pos = 11; //rt

						}
						// only increase pc if the token is an instruction
						pc +=0x1;
					}
					else if (type == 1) {
						// it is a register
						get_register_binary(token, one_line_instr,pos);

						if (strncmp(mem_access_register, token, 2) == 0) {
							save_mem_value = 1;
						}

						if (instr_type == 1) {
							if (strncmp(one_line_instr + 26, "000000", 6) == 0 || strncmp(one_line_instr +26, "000010", 6) ==0 ) {
								if (num_param == 3) pos = 11;
								else if (num_param == 2) {
									strncpy(one_line_instr + 6, "00000", 5); // fiill up $rs  
									pos = 21;
								}
							} else {
								if (num_param == 3) pos = 6;
								else if (num_param ==2) {
									pos = 11;
									strncpy(one_line_instr + 21, "00000",5);//shamt
								} 
							}
						}

						else if (instr_type == 2 ){

							if (num_param == 2) pos = 6;
							else if (num_param ==1) pos = 16;
						} 
						else if (instr_type == 5) {
							if(num_param == 2) pos = 11;
							else if (num_param ==1) pos = 16;
						}
						else if (instr_type == 4) {
							if (two_instr_addr == pc - 0x2) {
								// lui should be handled by above get_register_binary
								// need to fill for ori
								pos = 11;
								strncpy(one_line_instr+pos+32, one_line_instr+pos, 5); //rt
								strncpy(one_line_instr+6 + 32, one_line_instr+pos, 5); //rs

							}
							pos = 16;
						} 

						num_param--;
					}
					else if (type == 2) can_write = 0;
					else if (type == 3) {
						// it is a variable (do type conversion  
						if (is_text_section) {
							if (instr_type == 1) {
								// do conversion , save at pos 21
								if((int)strlen(token) == 1){} //empty token.. 
								else {
									dec_to_binary(atoi(token), one_line_instr+pos, 5);
								}
							}
							else if (instr_type == 2) {
								// for I-instrunctions, variables are sign extend or zero extend??
								if (is_memory_access) {
									// 8($10)
									if ((int)strlen(token) != 1) {
										parsememory(token, one_line_instr); 
									}
								} else { // just plain numbers 
									pos = 16;
									extend_bits(token, one_line_instr + pos, sign_extend, 16);

								}
							} 
							else if (instr_type ==5 ) { // bne or beq
								int val;
								char * bin = (char *) malloc(sizeof(char) *16);
								val = get_addr(token, head); // address of the label to jump to
								int next_instr_pc = pc;
								int diff = (val - next_instr_pc);
								dec_to_binary(diff, bin, 16);
								pos = 16;
								strncpy(one_line_instr+pos, bin, 16);
								free(bin);
							}
							else if (instr_type ==4) { // la instructions
								// data1 -> get address of data1
								// for lui 
								int val;
								char * bin = (char *) malloc(sizeof(char) * 32); 
								val = get_addr(token, head); 
								dec_to_binary(val, bin, 32);
								strncpy(one_line_instr+pos, bin, 16); // pos = 21, copy first 16 bits of address of current variable (Ex data1)
								// for ori
								if (two_instr_addr == pc - 0x2) {
									strncpy(one_line_instr+pos+32, bin+16, 16);
								}
								free(bin);
							}
							else if (instr_type == 3) { // jump instructions
								if((int)strlen(token) == 1){} //empty token.. 
								else {
									int val;
									char * bin = (char *) malloc(sizeof(char) * 26);
									val = get_addr(token, head);
									//int next_instr_pc = pc;
									//int diff = val - next_instr_pc;
									dec_to_binary(val, bin, 26);
									pos = 6;
									strncpy(one_line_instr+pos, bin, 26);
									free(bin);
								}
							}
						}
					}
					else {
						printf("not a recognized instruction\n");
					}

					token = strtok(NULL, " \t"); // get next token
				}
				if (can_write){
					printf("this is line is %s\n", one_line_instr);
					printf("instr length is %d\n", (int) strlen(one_line_instr));
					if ((int) strlen(one_line_instr) > 32 || (int) strlen(one_line_instr) < 32) {
						printf("WARNING the instruction length is not 32 : %d\n",(int) strlen(one_line_instr));
					}
					// instruction binary written first and then data binary....
					write_to_file(fp2, one_line_instr, totallen);	
				}
				// reset pos for next line of instruction
				pos = 0; 
				num_param = 0;
				is_memory_access = 0;
				save_mem_value = 0;
				can_write = 0;
				totallen = 32;
				// write binary_instruction to .o file
				free(one_line_instr);
			}

		} // end of second pass

		printf("num var %d\n", num_global_var);
		struct global_variable *d;
		while (num_global_var != 0) {
			char * dval = (char *) malloc(sizeof(char) * 32);
			d = pop_last(head);
			printf("%s\n", d->name);
			printf("%d\n", d->num_array_elem);
			if (d->num_array_elem > 1) {
				dval = (char *) realloc(dval,sizeof(char) * 32 * d->num_array_elem);
			}
			strncpy(dval, d->value, 32* d->num_array_elem);	
			printf("%s\n", dval);
			write_to_file(fp2, dval,32* d->num_array_elem);
			num_global_var --;
			free(dval);
		}


	}
	return 0;
}

int write_to_file(FILE * writefile, char * one_line_instr, int totallen){
	int i;
	for (i=0; i<totallen;i++) {
		fprintf(writefile, "%c", one_line_instr[i]);
	}

	return 0;
}


