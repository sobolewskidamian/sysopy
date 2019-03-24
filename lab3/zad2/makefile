CC = gcc -Wall

all: compile run clean
test: compile makeTest clean

compile:
	$(CC) main.c -o main
	$(CC) tester.c -o tester

run:
	./main /home/damian/Pulpit/Sysopy/lab3/zad2/pliktest 15 TRYB1
	
makeTest:
	./tester /home/damian/Pulpit/Sysopy/lab3/zad2/plik 1 3 100

clean:
	rm -f *.o
	rm -f main
	rm -f tester
