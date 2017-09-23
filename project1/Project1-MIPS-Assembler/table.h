#ifndef TABLE
#define TABLE

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

struct global_variable {
	char name[50];
	char value[256];
	int num_array_elem;
	int addr;
	int is_data;
	struct global_variable * next_variable;
};

struct global_variable * insert_variable(char * name_token, struct global_variable *head, int addr, int is_data);

void set_value_heap(char * token,int num_array_elem, int numbits, struct global_variable * head, int is_sign_extended);

int get_value(char * token, struct global_variable * head);
struct global_variable * pop_last(struct global_variable * head);

#endif
