#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "assembler.h"
#include "linkLoader.h"


/******************* program addr *****************************/
int ProgAddr(char addr[][COMMANDSIZE]){
	//function to change program start address
	prog_addr = HexToDec(addr[0]);
	return 0;
}
/************************* Run *******************************/
int Run(){
	
	return 0;
}


/************************ Break Point ***********************/
int BreakPoint(char par[][COMMANDSIZE]){

	return 0;
}



/*********************** Loader *****************************/
int LoaderPass1(char obj_file[][COMMANDSIZE]){
	int i, endflag=0;
	FILE *obj_p;
	char str[MAX_LINESIZE];
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
				case 'E'://end
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

		//initialize and check file name
		InitRefTab();
		if((estab[i].ctrl_sec)[0] =='\0') break;
		fp = fopen(obj_file[i],"r");
		
		//read 1 line string from .obj file
		while(fgets(str,MAX_LINESIZE,fp)){
			str[strlen(str)-1] = '\0';

			//record
			switch(str[0]){
				case 'R':	//external referance
					if(GetRefRec(str,i)==-1)
						return -1;
					break;
				case 'T':	//external definition
					if(GetTextRec(str, i) == -1)
						return -1;
					break;
				case 'M':	//modify
					if(GetModiRec(str, i) == -1)
						return -1;
					break;
				case 'E':	//end 
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
	return 0;
}
