#include "table.h"
#include "parseline.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// .word : numbits ==32
struct global_variable * insert_variable(char * name_token, struct global_variable *head, int addr, int is_data) {
	struct global_variable *new_var = (struct global_variable *) malloc(sizeof(struct global_variable));
	strcpy(new_var->name, name_token);
	new_var->next_variable = head;
	new_var->addr= addr;
	new_var->num_array_elem = 0;
	new_var->is_data = is_data;
	head = new_var;
	//	strncpy(*currpos, value_token, numbits);
	return new_var;
}

void set_value_heap (char * token,int num_array_elem, int numbits, struct global_variable * head, int is_sign_extended) {
	struct global_variable * curr = head; //head is the most recently inserted one so it is the current one to insert value to
	//	extend_bits(token, 
	int array_index = num_array_elem -1;
	char * temp = (char *) malloc(numbits * sizeof(char));

	extend_bits(token, temp, is_sign_extended, numbits); 
	strncpy(head->value+ (array_index *32),temp, numbits);
	head->num_array_elem = num_array_elem;
	//printf("head->name %s\n", head->name);
	//printf("head->value %s\n", head->value + array_index);
}



int get_addr (char * token, struct global_variable * head) {
	struct global_variable * curr = head;
	int tokenlen= (int) strlen(token);
	for (curr; curr != NULL; curr= curr->next_variable) {
		// variables in text section (here tokens) has a space char at the end FML
		if (strncmp (token, curr->name, tokenlen-1) == 0) {
			return curr->addr; 
		}
	}

	return 0;	
}

struct global_variable * pop_last(struct global_variable * head) {
	if (head == NULL) return NULL;
	struct global_variable * currelem = head;
	struct global_variable * nextelem = head->next_variable;
	struct global_variable *returnelem;
	if (nextelem == NULL) {
		//remove currelem
		returnelem = currelem;
		head = NULL;
		return returnelem;
	}
	while (nextelem->next_variable != NULL) {
		currelem = nextelem;
		nextelem = nextelem->next_variable;
	}
	returnelem = nextelem;
	currelem->next_variable = NULL;
	return returnelem;

}
