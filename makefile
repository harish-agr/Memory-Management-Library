
#Thisnext linedefines arguments that are passed to all compilation steps

CCFLAGS = -g -Wall -lpthread -pthread -std=c99

mem: mem.c memLibrary.h memLibrary.c
	gcc $(CCFLAGS) -o mem mem.c memLibrary.h memLibrary.c

