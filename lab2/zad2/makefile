CC = gcc -Wall

all: compile run clean

compile:
	$(CC) main.c -o main

run:
	./main /home/damian/Pulpit/abc "<" "2019-03-18 17:59:00" | tee -a wynik

clean:
	rm -f *.o
	rm -f main
