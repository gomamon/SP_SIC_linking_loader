#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
		memset(estab[i].ctrl_sec,'\0',10);
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
	char str[8]; // get intput string
	
	fscanf(fp,"%c",&in);
	if(in != 'H') return -1;

	for(i=0;i<6;i++) fscanf(fp,"%c",&(estab[cnt].ctrl_sec[i]));
	estab[cnt].ctrl_sec[6] = '\0';
	for(i=0 ; i<cnt; i++){
		if(!strcmp(estab[i].ctrl_sec, estab[cnt].ctrl_sec)){
			printf("Duplication External Symbol!\n");
			return -1;
		}
	}	
	
	for(i=0;i<6;i++) fscanf(fp,"%c",&(str[i]));
	str[6] = '\0';
	if(IsHex(str)){
		estab[cnt].addr = strtol(str,NULL,16)+ prog_addr;
	}

	for(i=0;i<6;i++) fscanf(fp,"%c",&(str[i]));
	str[6] = '\0';
	if(IsHex(str)){
		estab[cnt].len = strtol(str,NULL,16);
	}
	fscanf(fp,"%c",&in);

};

int MakeExtSymNode(char str[],int cnt){
	int i;
	char str_addr[MAX_LINESIZE];
	est_node *new_node;
	
	for(i=1; i<strlen(str); i+=12){
		new_node = (est_node*)malloc(sizeof(est_node));
		memset(new_node->extsym,'\0', 10); 
		new_node->next = NULL;

		strncpy(new_node->extsym, &(str[i]), 6);
		strncpy(str_addr, &(str[i+6]), 6);
		new_node->addr = strtol(str_addr,NULL,16);
	}

}
