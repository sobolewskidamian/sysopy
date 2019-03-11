CC = gcc -Wall
ARGUMENTS = create_table 3 search_directory / \\* temp1 search_directory /usr \\* temp2 search_directory /usr/bin \\* temp3 remove_block 0 remove_block 1 remove_block 2
FILE = raport3.txt

all: static dynamic shared

static:
	$(CC) -c library.c
	ar rcs library.a library.o
	$(CC) main.c -o main library.a -I .
	echo "------------------------------------\n|---------------STATIC-------------|\n------------------------------------" 1>> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	make clean

dynamic:
	$(CC) -fPIC -c library.c -o library.o
	$(CC) -fPIC -shared library.o -o library.so
	$(CC) -o main main_dynamic.c -I . -ldl
	echo "\n\n------------------------------------\n|--------------DYNAMIC-------------|\n------------------------------------" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	make clean


shared:
	$(CC) -fPIC -c library.c -o library.o
	$(CC) -fPIC -shared -o library.so library.o
	$(CC) main.c -o main -L. library.o -I .
	echo "\n\n------------------------------------\n|--------------SHARED--------------|\n------------------------------------" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	make clean

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.a
