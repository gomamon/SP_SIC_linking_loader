#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "assembler.h"
#include "linkLoader.h"

//struct to save reference symbol and number
struct REFERNum{
	int num;
	char refer_sym[7];
}reftab[100];
enum REGSTER{
	A=0, X=1, L=2, B=3, S=4, T=5, F=6, PC=8, SW=9
};
/************************** Break Point  ************************/
void PrintBP(){
	//print break point
	bp *np;
	printf("\tbreakpoint\n");
	printf("\t-----------\n");
	for(np=bphead ; np!=NULL; np=np->next){
		printf("\t%04X\n",np->addr);
	}
}

void InitBP(){
	//initial break point
	bp *np,*dp;
	np = bphead;

	while(1){
		if(np == NULL) break;
		dp = np;
		np = np->next;
		free(dp);
	}
	bphead = NULL;
	bprear = NULL;
}

void MakeBP(int addr){
	//make break point and link each nodes
	bp *newbp;
	
	//initial breakpoint nodes
	newbp = (bp*)malloc(sizeof(bp));
	newbp->next = NULL;
	newbp->addr = addr;
	
	//link nodes
	if(bphead==NULL) bphead = newbp;
	else bprear->next = newbp;
	bprear = newbp;
}


/************************** Loader *********************/
int CheckObj(char filename[]){
	//This function checks that file is vaild obj file.
	//If vaild, return 0. If not, return -1
	
    int namesize;
	FILE *obj_p;
	namesize = strlen(filename);
	if(strcmp(".obj",&filename[namesize-4])){
		printf("No object file\n");
		return -1;
	}

	if( (obj_p = fopen(filename, "r")) == NULL){
		printf("%s No exist\n",filename);
		return -1;
	}
	fclose(obj_p);	
	return 0;
}
void InitEST(){
	// initailize external sybol table
	
	int i;
	est_node* del,*bef;

	for(i=0 ; i<4; i++){

		//initiailze data
		memset(estab[i].ctrl_sec,'\0',7);
		estab[i].len = 0;
		estab[i].addr = 0;

		//free linked node and initialize pointer
		if(estab[i].next == NULL){
			estab[i].rear = NULL;
			continue;
		}
		for(del=estab[i].next ; del !=NULL; ){
			bef = del;
			del = del->next;
			free(bef);
		}
		estab[i].next = NULL;
		estab[i].rear = NULL;
	}
}
int GetHeaderRec(FILE *fp,int cnt){
	//Get Head reacord data(control section, start address, length)
	
	int i;
	char in; // get input from .obj file
	char str[7]; // get intput string
	
	fscanf(fp,"%c",&in);
	if(in != 'H') return -1;	//check 'H'record

	//get control section name
	for(i=0;i<6;i++) fscanf(fp,"%c",&(estab[cnt].ctrl_sec[i]));	

	//match format
	for(i=0; i<6; i++){
		if(!( isalnum(estab[cnt].ctrl_sec[i]) || isalpha(estab[cnt].ctrl_sec[i]) ) ){
			estab[cnt].ctrl_sec[i] = '\0';
			break;
		}
	}
	estab[cnt].ctrl_sec[6] = '\0';
	
	//check dupication
	for(i=0 ; i<cnt; i++){
		if(!strcmp(estab[i].ctrl_sec, estab[cnt].ctrl_sec)){
			printf("Duplication External Symbol!\n");
			return -1;
		}
	}	
	
	//start address
	for(i=0;i<6;i++) fscanf(fp,"%c",&(str[i]));
	str[6] = '\0';

	if(IsHex(str)){
		if(cnt != 0) estab[cnt].addr = (estab[cnt-1].addr + estab[cnt-1].len);
		else	estab[cnt].addr = prog_addr + strtol(str,NULL,16);
	}

	//length
	for(i=0;i<6;i++) fscanf(fp,"%c",&(str[i]));
	str[6] = '\0';
	
	if(IsHex(str)){
		estab[cnt].len += strtol(str,NULL,16);
	}
	fscanf(fp,"%c",&in);
	return 0;
}

int GetDefineRec(char str[], int cnt){
	//Process definition record data 
	//and fill external symbol table
	
	est_node *np;
	int i,j;
	int addr;
	char sym[MAX_LINESIZE];
	char str_addr[MAX_LINESIZE];


	for(i=1; i<strlen(str); i+=12){
		strncpy(sym, &(str[i]), 6);	//get 6 char to save external sym
		
		//match format
		for(j=0; j<6; j++){
			if( isalnum(sym[j]) && isalpha(sym[j]) )
				continue;
			sym[j] = '\0';
			break;
			
		}

		//duplication external symbol check
		for(j=0; j<=cnt ; j++){
			np = estab[j].next;
			while(1){
				if(np == NULL) break;
				if(!strcmp(sym, np->extsym) ){
					printf("ERROR:duplication external symbol!");
					return -1;
				}
				np = np->next;
			}
		}

		strncpy(str_addr, &(str[i+6]), 6);			//get 6 char to save address
		addr = strtol(str_addr,NULL,16);
		addr += estab[cnt].addr;	//change char(hex) to dec value and store 

		MakeExtSymNode(sym, addr, cnt);
	}
	return 0;
}


int MakeExtSymNode(char sym[],int addr,int cnt){
	est_node *new_node;
	
	new_node = (est_node*)malloc(sizeof(est_node));
	memset(new_node->extsym,'\0', 7); 
	new_node->addr = 0;
	new_node->next = NULL;

	strcpy(new_node->extsym,sym);
	new_node->addr = addr;

	if(estab[cnt].next==NULL) estab[cnt].next = new_node;	
	else	(estab[cnt].rear)->next = new_node;
	estab[cnt].rear = new_node;

	return 0;
}

void PrintEST(){
	//Print external symbol table
	int i;
	est_node *np;
	prog_len=0;

	printf("control\t\tsymbol\t\taddress\t\tlength\n");
	printf("section\t\tname\n");
	printf("-------------------------------------------------------\n");
	for(i=0 ; i<3 ; i++){
		if(estab[i].ctrl_sec[0] == '\0') break;
		printf("%s\t\t\t\t%04X\t\t%04X\n",estab[i].ctrl_sec,estab[i].addr,estab[i].len); 
		prog_len += estab[i].len;

		np = estab[i].next;
		while(1){
			if(np==NULL) break;
			printf("\t\t%s\t\t%04X\n",np->extsym,np->addr);
			np = np->next;
		}
	}

	printf("-------------------------------------------------------\n");
	printf("\t\t\t\t\ttotal length %04X\n",prog_len);
}


void InitRefTab(){
	//initialization reference table
	int i;
	for(i=0; i<100; i++){
		reftab[i].num = 0;
		memset(reftab[i].refer_sym, '\0',7);
	}
}



int GetRefRec(char str[], int cnt){
	//Get reference record and save data in reftab
	int i,j,n;
	char sym[7];
	char num[2];

	//initialization
	estab[cnt].ref_cnt = 1;

	memset(sym,'\0',7);
	memset(num,'\0',2);

	//read reference record
	for(i=1; i<strlen(str); i+=8){
		strncpy(num, &str[i], 2);
		num[2] = '\0';
		strncpy(sym, &str[i+2], 6);
		sym[6] = '\0';
		//match format
		for(j=0; j<7; j++){
			if(isalnum(sym[j]) && isalpha(sym[j]))
				continue;
			sym[j] = '\0';
			break;
		}

		n = strtol(num, NULL, 16);
		reftab[n].num = n;

		strcpy(reftab[n].refer_sym, sym);

		(estab[cnt].ref_cnt)=n;
	}
	return 0;
}



int GetTextRec(char str[], int cnt){
	int i;
	char addr_str[7];
	char data[3];
	int addr,len;

	//printf("ttt");
	strncpy(addr_str, &(str[1]), 6);//get 6 char to save external sym
	addr_str[6]='\0';
	addr = strtol(addr_str,NULL,16);
	addr += estab[cnt].addr;
	

	strncpy(data, &(str[7]), 2);
	data[2] = '\0';
	len = strtol(data, NULL ,16);

	for(i=0; i<len; i++){
		strncpy(data,&(str[9+(i*2)]),2);
		data[2] = '\0';
		strcpy(mem[addr+i],data);
	}
	return 0;
}

int FindExtSymAddr(char str[]){
	//Find external Symbol and return that address
	//If that symbol is not exist, return -1
	int i;
	est_node *np;

	for(i=0; i<3 ; i++){
		np = estab[i].next;
		while(1){
			if(np==NULL) break;
			if(!strcmp(np->extsym,str))
				return np->addr;
			np = np->next;
		}
	}
	return -1;
}

int GetModiRec(char str[], int cnt){
	//Read modification record and modify memory

	char addr_str[7];//modification address
	char mod_str[7]; //data to be modified
	char halbit_str[4];//number of half bits modified
	char ref_num_str[4]; //referecne symbol number
	char *after_mod;
	int addr, mod, halbit, ref_num, refer_addr;


	//string token to get address, half bit and reference symbol number 
	strncpy(addr_str, &(str[1]), 6);//get 6 char to save external sym
	addr_str[6] = '\0';
	strncpy(halbit_str, &(str[7]), 2);	//get 2 char to save modifi sym
	halbit_str[2] = '\0';
	strncpy(ref_num_str, &(str[9]), 3);
	ref_num_str[3] = '\0';

	addr = strtol(addr_str, NULL, 16);
	halbit = strtol(halbit_str, NULL, 16);
	ref_num = strtol(&(ref_num_str[1]), NULL, 16);

	//get reference external address
	if(ref_num == 1)	refer_addr = estab[cnt].addr; 
	else{	
		refer_addr = FindExtSymAddr(reftab[ref_num].refer_sym);
		if(refer_addr == -1){
			printf("No exist %s\n", str);//reftab[ref_num].refer_sym);
			return -1;
		}
	}
	addr += estab[cnt].addr;
	memset(mod_str,'\0',7);

	//reference number error
	if(ref_num > estab[cnt].ref_cnt){
		printf("modified error\n");
		return -1;
	}

	//half bit is 5
	else if(halbit%2 == 1){
		//get data from memory
		mod_str[0] = mem[addr][1];		
		mod_str[1] = mem[addr+1][0];	mod_str[2] = mem[addr+1][1];
		mod_str[3] = mem[addr+2][0];	mod_str[4] = mem[addr+2][1];
		mod_str[5] = '\0';

		//modify
		mod = strtol(mod_str, NULL , 16); 

		if(ref_num_str[0] == '-') refer_addr *= (-1);
		mod += refer_addr;
		
		//store modified data
		after_mod = DecToHex(mod, halbit);
		mem[addr][1] = after_mod[0];
		mem[addr+1][0] = after_mod[1]; mem[addr+1][1] = after_mod[2];
		mem[addr+2][0] = after_mod[3]; mem[addr+2][1] = after_mod[4];
	}
	//half bit is 6
	else{
		//get data from memory
		mod_str[0] = mem[addr][0];	mod_str[1] = mem[addr][1];
		mod_str[2] = mem[addr+1][0];	mod_str[3] = mem[addr+1][1];
		mod_str[4] = mem[addr+2][0];	mod_str[5] = mem[addr+2][1];
		mod_str[6] = '\0';

		//modify
		mod = strtol(mod_str, NULL ,16);

		if(ref_num_str[0] == '-') refer_addr *= (-1);
		mod += refer_addr;

		//store modified data
		after_mod = DecToHex(mod, halbit);
			
		mem[addr][0] = after_mod[0]; mem[addr][1] = after_mod[1];
		mem[addr+1][0] = after_mod[2]; mem[addr+1][1] = after_mod[3];
		mem[addr+2][0] = after_mod[4]; mem[addr+2][1] = after_mod[5];
		
	}

	if(after_mod!=NULL)
		free(after_mod);
	return 0;
}

char* DecToHex(int dec,int size){
	//function to chage decimal int value to hexadecimal string
	char *hex= (char*)malloc(sizeof(char) * size + 1);
	int i=0,j;
	int max = 1;

	//process big number
	for(i=0; i<=size; i++) max *= 16;
		dec %= max;
	memset(hex,'\0',size+1);
	if (dec<0)	//process negative num
		dec= max + dec;

	for(i=0; i<size; i++){
		//shift right
		if(i>0){
			for(j=size-2; j >= 0; j--)	hex[j+1] = hex[j];
		}
		hex[0] = ( 0 <= dec%16  && dec%16<=9 ) ? ((dec%16)+'0') : ((dec%16%10) +'A') ;
		dec/=16;
	}
	return hex;
}
/************************** Run ************************/
int SearchBP(int addr,int form){
	//search break point
	//If find break point, then return the address
	//If no break point at address, then return 0
	bp* np;
	for(np=bphead ; np!=NULL; np=np->next){
		if(np->addr>=addr && np->addr<=addr+form){
			return np->addr;
		}
	}
		
	return -1;
}
void PrintReg(){
	//print all register
	printf("\tA : %012X X : %08X\n",reg[A], reg[X]);
	printf("\tL : %012X PC: %012X\n",reg[L], reg[PC]);
	printf("\tB : %012X S : %012X\n",reg[B], reg[S]);
	printf("\tT : %012X\n",reg[T]);
	
}

int ExcuteInst(int addr, int form){
	
	//excute instruction by format
	unsigned int data=0;
	int opcode;
	int r1, r2;

	switch(form){
		case 1:	
			//get data in format 1
			data = strtol(mem[addr],NULL,16);
			opcode = data;
			switch(opcode){
				case FIX:
					reg[A] = reg[F];
					break;
				case FLOAT:
					reg[F] = reg[A];
					break;
				case HIO:
				case SIO:
				case TIO:
					break;
			}
			return 1;
			break;
		case 2:
			//format 2
			data = strtol(mem[addr],NULL,16)*0x100;
			data += strtol(mem[addr+1],NULL,16);
			opcode = data & 0xFF00;
			r1 = data & 0x00F0;
			r2 = data & 0x000F;
			switch(opcode){
				case ADDR:
					reg[r2] =reg[r2] + reg[r1]; 
					break;
				case CLEAR:
					reg[r1] = 0;
					break;
				case COMPR:
					flagC = (reg[r1] == reg[r2]) ? 1 : 0;
					break;
				case DIVR:
					reg[r2] = reg[r2] / reg[r1];
					break;
				case MULR:
					reg[r2] = reg[r2] * reg[r1];
					break;
				case RMO:
					reg[r2] = reg[r1];
					break;
				case SHIFTL:
					reg[r1] = (reg[r1]*2) % 0x100000;
					break;
				case SUBR:
					reg[r2] = reg[r2] - reg[r1];
					break;
				case SVC:
					break;
				case TIXR:
					reg[1] = reg[1] + 1;
					flagC = (reg[1] == reg[r1]) ? 1 : 0;
					break;
				default:
					return -1;
			}
			return 2;
			break;
			/*
		case 3:
			data = strtol(mem[addr],NULL,16)*0x10000;
			data += strtol(mem[addr+1],NULL,16)*0x100;
			data += strtol(mem[addr+2],NULL,16);
			opcode = data & 0xFC0000;
			x = data & 0x008000;

			break;
		case 4:			
			data = strtol(mem[addr],NULL, 16)*0x1000;
			data += strtol(mem[addr+1],NULL,16)*0x100;
			data += strtol(mem[addr+2],NULL,16)*0x10;
			data += strtol(mem[addr+3],NULL,16);
			opcode = data & 0xFC000000;
			x = data & 0x00800000;
			
			break;*/
		default:
			break;
	}
	return 1;
}




