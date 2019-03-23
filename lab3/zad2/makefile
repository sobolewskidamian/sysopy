CC = gcc -Wall

all: compile run clean

compile:
	$(CC) main.c -o main

run:
	./main /home/damian/Pulpit/Sysopy/lab3/zad2/pliktest 15 TRYB2
	
test:
	make compile
	./main RUN_TESTER /home/damian/Pulpit/Sysopy/lab3/zad2/plik 1 3 100
	make clean

clean:
	rm -f *.o
	rm -f main
