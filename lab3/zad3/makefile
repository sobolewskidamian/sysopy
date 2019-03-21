CC = gcc -Wall

all: compile run clean

compile:
	$(CC) main.c -o main

run:
	./main /home/damian/Pulpit/Sysopy/lab3/zad3/plik 15 tryb2
	
test:
	./main RUN_TESTER /home/damian/Pulpit/Sysopy/lab3/zad3/plik 1 3 100

clean:
	rm -f *.o
	rm -f main
