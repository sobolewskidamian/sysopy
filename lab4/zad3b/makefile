CC = gcc -Wall

all: compile run clean

compile:
	$(CC) sender.c -o sender
	$(CC) catcher.c -o catcher

run:
	./catcher 500 SIGQUEUE

clean:
	rm -f *.o
	rm -f sender
	rm -f catcher
