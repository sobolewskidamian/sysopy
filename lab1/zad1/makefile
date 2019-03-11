CC = gcc -Wall

all: static shared

static:
	$(CC) -c library.c
	ar rcs library.a library.o
	make clean

shared:
	$(CC) -c -fPIC library.c
	$(CC) -shared -fPIC -o library.so library.o
	make clean

clean:
	rm -f *.o

