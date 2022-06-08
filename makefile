main:	main.o
	gcc  -o mysh main.o
	rm main.o
main.o:	main.c file.h 
	gcc -c main.c
file.h: getargs.h
