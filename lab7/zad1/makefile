CC = gcc -Wall -pedantic -std=gnu11

all: trucker.out loader.out

trucker.out: trucker.c data.o
	$(CC) -c $<
	$(CC) -o $@ data.o trucker.o

loader.out: loader.c data.o
	$(CC) -c $<
	$(CC) -o $@ data.o loader.o

data.o: data.c data.h
	$(CC) -c $<

clean:
	rm -f *.o *.a *.so.* *.out *.temp loader trucker
