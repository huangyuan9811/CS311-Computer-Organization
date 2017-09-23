#include "parseline.h"
#include "table.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


// check if token is an instruction, section,  register or something else (ex instruction)
int token_type(char * token, char * one_line_instr, int *sign_extend) {
	if (token != NULL) {
		if (strncmp(token, ".data",5) == 0) {
			return 10;
		}
		else if (strncmp(token, ".word",5 ) ==0) {
			return 11;
		}
		else if (strncmp(token, ".text", 5) ==0) {
			return 12;
		}
		else if (get_instr_binary(token, one_line_instr, sign_extend) > -1){
			return 0;
		}
		else if (token[0] == '$') {
			return 1;
		}
		else if(strncmp(token + strlen(token)-1 , ":", 1)==0) {
			return 2;
		}
		else if(strncmp(token + strlen(token)-2, ":", 1) ==0 ){
			return 2;
		}
		else {
			return 3;
		}
	}
	return -1;
}

int get_register_binary(char * token, char * one_line_instr, int pos) {
	int i;
	// array to store the string rep of decimal number, initialized to 1 digit
	//char *decimal = (char *) malloc (sizeof(char));
	// check number of digits 
	int count=0;
	// each char rep of decimal number
	char decimalchar;

	// loop through the token to only extract the string rep of decimal number (remove '$' and 's')
	if (token[strlen(token)-1] == ',') token[strlen(token)-1] = 0; 
	/*
	   for (i = 1; token[i] !=0 ; i++){
	   decimalchar = token[i];
	// check for invalid values first by converting decimalchar to int number
	int dec = decimalchar - '0';
	if (dec <0 || dec >9) {
	return -1;  // error
	}

	// if current char is 1st digit
	if (count==0) {
	strncpy(decimal, &decimalchar, 5);
	} 
	// if current char is 2nd digit, realloc to hold 2 digits
	else if(count ==1) {
	decimal = (char *) realloc(decimal, 2);
	strncpy(decimal+1, &decimalchar,5);
	}
	count++;

	}
	 */
	// convert string rep of decimal to actual decimal number
	int regnum = atoi(token+1);
	// convert int decimal to string binary and store in the correct position
	dec_to_binary(regnum, one_line_instr+pos, 5);
	//free(decimal);
	return 0;   
}

// R-type -> 1, I-type ->2 , J-type->3, la->4, branch -> 5
int get_instr_binary(char * token, char *one_line_instr, int *sign_extend) {
	*sign_extend = 0;
	if (strcmp(token, "addiu") == 0) {
		strncpy(one_line_instr, "001001", 6);
		*sign_extend = 1;
		return 2;}
	else if (strcmp(token, "addu") == 0) {
		strncpy(one_line_instr, "000000", 6);
		strncpy(one_line_instr + 26 , "100001",6);
		return 1;}
	else if (strcmp(token, "and") == 0) {
		strncpy(one_line_instr, "000000",6);
		strncpy(one_line_instr+26, "100100",6);
		return 1;}
	else if (strcmp(token, "andi") == 0) {
		strncpy(one_line_instr, "001100",6);
		return 2;}
	else if (strcmp(token, "beq") == 0) {
		strncpy(one_line_instr, "000100",6);
		*sign_extend =1;
		return 5;}
	else if (strcmp(token, "bne") == 0) {
		strncpy(one_line_instr, "000101",6);
		*sign_extend =1;
		return 5;}
	else if (strcmp(token, "j") == 0) {
		strncpy(one_line_instr, "000010",6);
		return 3;}
	else if (strcmp(token, "jal") == 0) {
		strncpy(one_line_instr, "000011",6);
		return 3;}
	else if (strcmp(token, "jr") == 0) {
		strncpy(one_line_instr, "000000",6);
		strncpy(one_line_instr + 26 , "001000",6);
		return 1;}
	else if (strcmp(token, "lui") == 0) {
		strncpy(one_line_instr, "001111",6);
		return 2;}
	else if (strcmp(token, "lw") == 0) {
		strncpy(one_line_instr, "100011",6);
		*sign_extend =1;
		return 2;}
	else if (strcmp(token, "la") == 0) {
		return 4;}
	else if (strcmp(token, "nor") == 0) {
		strncpy(one_line_instr, "000000",6);
		strncpy(one_line_instr + 26 , "100111",6);
		return 1;}
	else if (strcmp(token, "or") == 0) {
		strncpy(one_line_instr, "000000",6);
		strncpy(one_line_instr+26, "100101",6);
		return 1;}
	else if (strcmp(token, "ori") == 0) {
		strncpy(one_line_instr, "001101",6);
		return 2;}
	else if (strcmp(token, "sltiu") == 0) {
		strncpy(one_line_instr, "001011",6);
		*sign_extend =1;
		return 2;}
	else if (strcmp(token, "sltu") == 0) {
		strncpy(one_line_instr, "000000",6);
		strncpy(one_line_instr + 26, "101011",6);
		return 1;}
	else if (strcmp(token, "sll") == 0) {
		strncpy(one_line_instr, "000000",6);
		strncpy(one_line_instr + 26 , "000000",6);
		return 1;}
	else if (strcmp(token, "srl") == 0) {
		strncpy(one_line_instr, "000000",6);
		strncpy(one_line_instr + 26 , "000010",6);
		return 1;}
	else if (strcmp(token, "sw") == 0) {
		strncpy(one_line_instr, "101011",6);
		*sign_extend =1;
		return 2;}
	else if (strcmp(token, "subu") == 0) {
		strncpy(one_line_instr, "000000",6);
		strncpy(one_line_instr + 26 , "100011",6);
		return 1;}
	else {
		return -1;
	}

}


int dec_to_binary(int decimal, char *save, int numbits) {
	int i;
	//save[5] = '\0' ;
	char * temp = (char *) malloc(sizeof(char) * numbits);
	for (i = numbits-1; i >=0; --i, decimal >>=1) {
		temp[i] = (decimal & 1) + '0';
	}
	strncpy(save, temp, numbits);
	free(temp);
	return 0;
}

int hex_to_binary(int hex, char *save, int numbits, int signextend) {
	int i;
	int savehex = hex;
	int signedvalue;
	int numshift = 0;

	if (hex < 0x10 ){
		numshift = 32-4;
	} else if (hex < 0x100) {
		numshift = 32-8;
	} else if (hex < 0x1000) {
		numshift = 32-12;
	} else if (hex < 0x10000) {
		numshift = 32-16;
	}

	// if needed to sign extend, do bit shifting
	if (signextend) {
		signedvalue = (savehex << numshift) >> numshift;
		hex = signedvalue;
	}
	for (i = numbits-1; i>=0; --i) {
		save[i] = (hex & 1) + '0';
		hex >>=1;
	}
	return 0;
}

int extend_bits(char * token, char * save, int is_sign_extended, int goalbits) {
	// is it decimal or hex representation or global variable??
	if (is_decimal(token)) {
		dec_to_binary(atoi(token),save,goalbits); 
	} else {
		//int hex = atoi(token+2);
		int hex= (int) strtol(token, NULL, 16);
		hex_to_binary(hex, save, goalbits, is_sign_extended);
	}

	return 0;
}

int is_decimal (char * token) {
	if (strncmp(token, "0x", 2) == 0) {
		return 0;
	} else {
		return 1;
	}
}

int check_lower_16bit(int val) {
	char * bin = (char *) malloc(sizeof(char) * 32);
	dec_to_binary(val, bin,32); 
	if (strncmp (bin+16 , "0000000000000000", 16) == 0) {
		return 1; 
	}
	free(bin);
	return 0;
}

int parsememory(char * token, char *save) {
	char * offset_str;
	char * reg_str;
	int offset;
	offset_str = strtok(token, "(");
	offset = atoi(offset_str);
	token = strtok(NULL, " \t");
	printf("offset is %d\n", offset);
	reg_str = strtok(token, ")");
	printf("register is %s\n", reg_str);
	//	get_register_binary(reg_str, save, 6);
	dec_to_binary(atoi(token +1), save+6, 5);
	dec_to_binary(offset, save+16, 16);
	return 0;
}
