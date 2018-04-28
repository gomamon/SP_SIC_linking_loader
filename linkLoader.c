#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "assembler.h"
#include "linkLoader.h"



int ProgAddr(char addr[][COMMANDSIZE]){

	reg.pc = HexToDec(addr[0]);
	
	return 0;
}
int Run(){

	return 0;
}

int BreakPoint(char par[][COMMANDSIZE]){

	return 0;
}

int LoaderPass1(char obj_file[][COMMANDSIZE]){
	int i;
	FILE *obj_p;

	for(i=0; i<3; i++){
		if(obj_file[i][0]!='\0') break;
		if(CheckObj(obj_file[i])==-1){
			printf("No object file\n");
			return -1;
		}
		if( (obj_p = fopen(obj_file[i], "r")) == NULL){
			printf("%d\'s No exist\n",i);
			return -1;
		}
	}
	return 0;
}

int Loader_pass2(){

}
