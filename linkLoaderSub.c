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
	return 0;
}
