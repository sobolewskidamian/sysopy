CC = gcc -Wall
ARGUMENTS = create_table 3 search_directory / \\* temp1 search_directory /usr \\* temp2 search_directory /usr/bin \\* temp3 remove_block 0 remove_block 1 remove_block 2
FILE = raport2.txt

all: static shared

static:
	$(CC) -c library.c
	ar rcs library.a library.o
	$(CC) main.c -o main library.a -I .
	echo "------------------------------------\n|---------------STATIC-------------|\n------------------------------------" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	make clean

shared:
	$(CC) -c -fPIC library.c
	$(CC) -shared -fPIC -o library.so library.o
	$(CC) main.c -o main -L. library.o -I .
	echo "\n\n------------------------------------\n|--------------SHARED--------------|\n------------------------------------" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	make clean

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.a
