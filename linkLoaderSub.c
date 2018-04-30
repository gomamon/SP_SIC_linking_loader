#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "assembler.h"
#include "linkLoader.h"


int CheckObj(char filename[]){
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
	int i;
	est_node* del,*bef;

	for(i=0 ; i<4; i++){
		memset(estab[i].ctrl_sec,'\0',7);
		estab[i].len = 0;
		estab[i].addr = 0;
		if(estab[i].next == NULL) continue;
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
	int i;
	char in; // get input from .obj file
	char str[7]; // get intput string
	
	fscanf(fp,"%c",&in);
	if(in != 'H') return -1;


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

}

int GetDefineRec(char str[], int cnt){
	est_node *np;
	int i,j;
	int addr;
	char sym[MAX_LINESIZE];
	char str_addr[MAX_LINESIZE];


	for(i=1; i<strlen(str); i+=12){
		strncpy(sym, &(str[i]), 6);	//get 6 char to save external sym
		
		//match format
		for(j=0; j<6; j++){
			if(!( isalnum(sym[i]) || isalpha(sym[i]) ) ){
				sym[i] = '\0';
				break;
			}
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
	printf("\t\t\t\t\ttotal length %4X\n",prog_len);
}
int GetRefRec(char str[], int cnt){
	int i,j;
	char sym[7];
	char num[2];
	
	estab[cnt].ref_num = (ref_node*)malloc( (strlen(str)/8 + 2) * sizeof(ref_node) );

	for(i=1; i<strlen(str); i+=8){
		strncpy(num, &(str[i]), 2);	//get 6 char to save external sym
		strncpy(sym, &(str[i+2]), 6);	//get 6 char to save external sym
		
		//match format
		for(j=0; j<6; j++){
			if(!( isalnum(sym[i]) || isalpha(sym[i]) ) ){
				sym[i] = '\0';
				break;
			}
		}

		estab[cnt].ref_num[strtol(num,NULL,16)].num = strtol(num,NULL,16);
		strcpy(estab[cnt].ref_num[strtol(num,NULL,16)].refer_sym , sym);
	}
	return 0;

}

int GetTextRec(char str[], int cnt){
	int i;
	char addr_str[7];
	char data[3];
	int addr,len;

	strncpy(addr_str, &(str[1]), 6);	//get 6 char to save external sym
	addr = strtol(addr_str,NULL,16);
	addr += estab[cnt].addr;
	
	strncpy(data, &(str[7]), 2);
	len = strtol(data, NULL ,16);

	for(i=0; i<len; i++){
		strncpy(data,&str[9+(i*2)],2);
		strcpy(mem[addr+i],data);
	}
	return 0;
}

int FindExtSymAddr(char str[]){
	int addr;
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
	char addr_str[7];//modification address
	char mod_str[7]; //alternate address to modified
	char halbit_str[4];//number of half bits modified
	char ref_num[4]; //referecne symbol number
	int addr, mod, halbit, num, refer_addr;
	int i;
	for(i=1; i<strlen(str); i+=11){
		strncpy(addr_str, &(str[i]), 6);	//get 6 char to save external sym
		strncpy(halbit_str, &(str[i+6]), 2);	//get 2 char to save modifi sym
		strncpy(ref_num, &(str[i+8]), 3);

		addr = strtol(addr_str, NULL, 16);
		halbit = strtol(halbit_str, NULL, 16);
		num = strtol(&(ref_num[1]), NULL, 16);
		
		memset(mod_str,'\0',7);
		if(halbit%2 == 1){
			refer_addr = FindExtSymAddr(estab[cnt].ref_num[num].refer_sym);
			if(refer_addr == -1) return -1;
			if(ref_num[0] == '-') refer_addr *= (-1);
			mem[refer_addr][1] = mod_str[0];
			strcpy(&(mod_str[1]),mem[refer_addr+1]);
			strcpy(&(mod_str[3]), mem[refer_addr+2]);
		}
		else{
		

		}

	}

}

char* DecToHex(int dec,int size){
	//function to chage decimal int value to hexadecimal string
	char *hex= (char*)malloc(sizeof(char) * size);
	int i=0,j;
	memset(hex,'\0',8);
	for(i=0; i<size; i++){
		//shift right
		if(i!=0){
			for(j=i-1; j<0;j--){
				hex[j+1] = hex[j];
			}
		}
		hex[0] = (dec%16>='0' && dec%16<='9' ) ? dec%16+'0' : dec%16+'A' ;
		i++;
		dec/=16;
	}
	return hex;
}

