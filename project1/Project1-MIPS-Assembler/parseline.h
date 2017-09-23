#ifndef PARSELINE
#define PARSELINE

int token_type(char * token, char * one_line_instr, int * sign_extend);
int get_register_binary(char * token,char * one_line_instr,  int pos);
int get_instr_binary(char * token, char * one_line_instr, int *sign_extend);
int dec_to_binary(int decimal, char * save, int numbits);
int hex_to_binary(int hex, char * save, int numbits, int signextend);
int extend_bits(char * token, char * save, int sign_or_zero, int goalbits);
int check_lower_16bit(int val);
int is_decimal (char * token); 
int parsememory(char *token, char * one_line_instr);


#endif
