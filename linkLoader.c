#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "assembler.h"
#include "linkLoader.h"

//enum to mark register
enum REGISTER{
	A=0, X=1, L=2, B=3, S=4, T=5, F=6, PC=8, SW=9
};

//run flag
int run_start = -1;

/******************* program addr *****************************/
int ProgAddr(char addr[][COMMANDSIZE]){
	//function to change program start address
	prog_addr = HexToDec(addr[0]);
	return 0;
}



/************************ Break Point ***********************/
int BreakPoint(char par[][COMMANDSIZE]){
	//Make, print, remove break point
	
	if(par[0][0] == '\0'){ 
		PrintBP();//print all break point
	}
	else if(!strcmp(par[0],"clear")){
		InitBP();//delete all break point
		printf("\t[ok] clear all breakpoints\n");
	}
	else{
		MakeBP(strtol(par[0],NULL,16));	//make break point
		printf("\t[ok] create breakpoints %04X\n",(unsigned)strtol(par[0],NULL,16));
	}
	return 0;
}



/*********************** Loader *****************************/
int LoaderPass1(char obj_file[][COMMANDSIZE]){
	//Fill external symbol table
	//If they have error return -1
	
	int i, endflag=0;
	FILE *obj_p;
	char str[MAX_LINESIZE];
	InitEST();

	for(i=0; i<3; i++){
		//check file
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

	PrintEST(); //print external symbol table
	LoaderPass2(obj_file);
	return 0;
}

int LoaderPass2(char obj_file[][COMMANDSIZE] ){
	//Read reference, text and modification record
	//and load data to memory
	//if they have error return -1
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

/************************* Run *******************************/

int Run(){
	//Read and Excute by instruction in memory
	//But Incomplete :(
	int addr;

	if(run_start==-1){
		reg[PC] = prog_addr;//set PC register
		 addr = prog_addr;
	
	}
	else addr = run_start;
	//from program start address to finish
	while(addr <= prog_addr + prog_len){
		
		if(!strcmp(mem[addr],"00")){
			addr++;	
			continue;
		}

		switch(mem[addr][0]){
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case 'D':
			case 'E':
				if(mem[addr+1][0] % 2 == 1 ) reg[PC] +=  4;
				else reg[PC] +=  3;
				//if(mem[addr+1][0] % 2 == 1 ) addr += ExcuteInst(addr, 4);
				//else addr += ExcuteInst(addr, 3);
				break;
			case '9':
			case 'A':
			case 'B':
				switch(mem[addr][1]){
					case '0':
					case '4':
					case '8':
					case 'C':
						reg[PC] += 2;
						//ExcuteInst(addr,2);
						break;
					default:
						if(mem[addr+1][0] % 2 == 1 ) reg[PC] += 4;
						else reg[PC] +=  3;
						//if(mem[addr+1][0] % 2 == 1 ) addr += ExcuteInst(addr, 4);
						//else addr += ExcuteInst(addr, 3);
						break;
				}
				break;

			case 'C':
			case 'F':
				switch(mem[addr][1]){
					case '0':
					case '4':
					case '8':
					case 'C':
						reg[PC] += 1;
						//addr += ExcuteInst(addr,1);
						break;
					default:
						if(mem[addr+1][0] % 2 == 1 ) reg[PC] += 4;
						else reg[PC] += 3;
						//if(mem[addr+1][0] % 2 == 1 ) addr += ExcuteInst(addr, 4);
						//else addr += ExcuteInst(addr, 3);
						break;
				}
				break;
			default :
				reg[PC]++;
		}
		if(SearchBP(addr,reg[PC]-addr) != -1){
			//If exist break point at the address, print register
			PrintReg();
			printf("Stop at checkpoint[%04X]\n", SearchBP(addr,reg[PC]-addr)); 
			run_start = reg[PC];
			break;
		}
		addr = reg[PC];
	}
	if(addr >= prog_addr + prog_len){
		PrintReg();
		printf("End program.\n");
		run_start = -1;
	}
	return 0;
}
