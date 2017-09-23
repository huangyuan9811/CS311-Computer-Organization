#include<stdio.h>
#include<string.h>



int main() {
	FILE *myfile;
	FILE *answer;
	char * line1 = NULL;
	ssize_t len1 = 0;
	ssize_t read1;
	char * line2 = NULL;
	ssize_t len2 = 0;
	ssize_t read2;

	myfile = fopen("example5.o", "r");
	answer = fopen("Project1_examples/example5.o", "r");
	
	read1 = getline(&line1, &len1, myfile);
	read2 = getline(&line2, &len2, answer);
	printf("len myfile %d\n", (int) strlen(line1));
	printf("len answer %d\n", (int) strlen(line2));
	if (strncmp(line1, line2,(int)strlen(line1)) == 0) {
		printf("pass\n");
	} else {
		printf("fail\n");
	}
	
	return 0;
}
