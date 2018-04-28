20161622.out: 20161622.o assembler.o linkLoader.o linkLoaderSub.o
	gcc -Wall -g -o 20161622.out 20161622.o assembler.o linkLoader.o linkLoaderSub.o
20161622.o: 20161622.c 20161622.h main.h assembler.h linkLoader.h
	gcc -Wall -g -c 20161622.c -o 20161622.o
assembler.o: assembler.c assembler.h main.h 20161622.h 
	gcc -Wall -g -c assembler.c -o assembler.o
linkLoaderSub.o: linkLoaderSub.c main.h linkLoader.h assembler.h
	gcc -Wall -g -c linkLoaderSub.c -o linkLoaderSub.o
linkLoader.o: linkLoader.c main.h linkLoader.h assembler.h 
	gcc -Wall -g -c linkLoader.c -o linkLoader.o
clean: 
	-rm *.o
	-rm *.lst
	-rm 20161622.out
