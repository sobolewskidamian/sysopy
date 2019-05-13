CC = gcc -Wall -pedantic -std=gnu11
DLL = -lpthread -lrt

all: trucker.out loader.out

trucker.out: trucker.c data.o
	$(CC) -c $< $(DLL)
	$(CC) -o $@ data.o trucker.o $(DLL)

loader.out: loader.c data.o
	$(CC) -c $< $(DLL)
	$(CC) -o $@ data.o loader.o $(DLL)

data.o: data.c data.h
	$(CC) -c $< $(DLL)

clean:
	rm -f *.o *.a *.so.* *.out *.temp loader trucker
