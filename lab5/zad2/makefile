GCC = gcc -Wall

FNAME = "FF"

all:
	make compile
	./main $(FNAME) 3 3

compile:
	$(GCC) -o main main.c
	$(GCC) -o master master.c
	$(GCC) -o slave slave.c

runMaster:
	./master $(FNAME)

runSlave:
	./slave $(FNAME) 10
