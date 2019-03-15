CC = gcc -Wall

all: compile run clean

compile:
	$(CC) main.c -o main

run:
	./main /home/damian/Pulpit "<" "2019-03-14 14:02:00" | tee -a wynik

clean:
	rm -f *.o
	rm -f main
