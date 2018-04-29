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
		

		GetHeaderRec(obj_p, i); //get Head reacord
		while( fgets(str,MAX_LINESIZE,obj_p)){
			str[strlen(str)-1] = '\0';

			switch(str[0]){
				case 'D'://external definition
					GetDefineRec(str, i);
					break;
				case 'E'://end break;
					endflag=1;
					break;
				default :
					continue;
					break;
			}
			if(endflag){
				endflag = 0;
				break;
			}
		}
		fclose(obj_p);
		
	}

	PrintEST();
	LoaderPass2(obj_file);
	return 0;
}

int LoaderPass2(char obj_file[][COMMANDSIZE] ){
	int i,endflag=0;
	char str[MAX_LINESIZE];
	FILE *fp;

	for(i=0 ; i<3 ;i++){
		if((estab[i].ctrl_sec)[0] =='\0') break;
		fp = fopen(obj_file[i],"r");

		while(fgets(str,MAX_LINESIZE,fp)){
			str[strlen(str)-1] = '\0';
			switch(str[0]){
				case 'T'://external definition
					GetTextRec(str, i);
					break;
				case 'E'://end break;
					endflag=1;
					break;
				default :
					continue;
					break;
			}
			if(endflag){
				endflag = 0;
				break;
			}
		}
		fclose(fp);	
	}
}
