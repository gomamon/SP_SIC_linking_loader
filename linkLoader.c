#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "assembler.h"
#include "linkLoader.h"



int ProgAddr(char addr[][COMMANDSIZE]){
	prog_addr = HexToDec(addr[0]);
	return 0;
}

int Run(){

	return 0;
}

int BreakPoint(char par[][COMMANDSIZE]){

	return 0;
}

int LoaderPass1(char obj_file[][COMMANDSIZE]){
	int i, endflag=0;
	FILE *obj_p;
	char str[MAX_LINESIZE];
	char in;
	InitEST();

	for(i=0; i<3; i++){
		if(obj_file[i][0]=='\0') break;
		if(CheckObj(obj_file[i])==-1) return -1;
		obj_p = fopen(obj_file[i], "r");
		

		GetHeaderRec(obj_p, i);
		while( fgets(str,MAX_LINESIZE,obj_p)){
			str[strlen(str)-1] = '\0';
			switch(str[0]){
				case 'D':
					MakeExtSymNode(str, i);
					break;
				case 'E':
					endflag=1;
					break;
				default :
					continue;
			}
			if(endflag){
				endflag = 0;
				break;
			}
		}
		
	}
	return 0;
}

int Loader_pass2(){

}
