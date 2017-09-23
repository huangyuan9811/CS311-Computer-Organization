#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main (){
	char * bin = (char *) malloc ( 16 * sizeof(char ));
	//dec_to_binary(atoi("-5"), bin, 32);
	char * hexstring = "0xa";
	int hex = (int) strtol(hexstring, NULL, 16);
	printf("%x\n",hex);
	hex_to_binary(hex, bin, 16,0);
	printf("%s\n",bin);
	return 0;

}

int dec_to_binary(int decimal, char * save, int numbits) {
	int i;
	for (i = numbits-1; i>=0;--i, decimal >>=1) {
		save[i] = (decimal & 1) + '0';
	}
	return 0;

}

int hex_to_binary(int hex, char * save, int numbits, int signextend) {
	int i;
	int savehex = hex;
	int numshift = 0;
	
	if (hex < 0x10) {
		numshift = 32 - 4;
	} else if (hex < 0x100) {
		printf("here\n");
		numshift = 32 - 8;
	} else if (hex < 0x1000) {
		numshift = 32- 12;
	} else if (hex < 0x10000) {
		numshift = 32 - 16;
	}
	
	// sign extend 16 bits to numbits
	if (signextend) {
		int signedvalue;
		signedvalue = (savehex << numshift) >> numshift;
		printf("signedvalue %x\n", signedvalue);
		hex = signedvalue;	
	}
	for (i = numbits; i>=0 ;--i) {
		save[i] =( hex & 1 )+ '0';
		hex >>= 1;
	}

	return 0;

}
