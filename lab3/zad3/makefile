CC = gcc -Wall

all: compile run clean
test: compile makeTest clean

compile:
	$(CC) main.c -o main
	$(CC) tester.c -o tester

run:
	./main /home/damian/Pulpit/Sysopy/lab3/zad3/plik 15 TRYB1 20 20
	
makeTest:
	./tester /home/damian/Pulpit/Sysopy/lab3/zad3/plik 1 3 100

clean:
	rm -f *.o
	rm -f main
