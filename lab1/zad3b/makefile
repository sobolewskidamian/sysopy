CC = gcc -Wall
ARGUMENTS = create_table 3 search_directory / \\* temp1 search_directory /usr \\* temp2 search_directory /usr/bin \\* temp3 remove_block 0 remove_block 1 remove_block 2
FILE = raport3b.txt

all: static dynamic shared

static:
	$(CC) -c library.c
	ar rcs library.a library.o
	echo "------------------------------------\n|---------------STATIC-------------|\n------------------------------------" >> $(FILE)
	$(CC) main.c -o main library.a -I . -O
	echo "-----\n--O--\n-----\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) main.c -o main library.a -I . -O1
	echo "\n\n------\n--O1--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) main.c -o main library.a -I . -O2
	echo "\n\n------\n--O2--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) main.c -o main library.a -I . -O3
	echo "\n\n------\n--O3--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) main.c -o main library.a -I . -O0
	echo "\n\n------\n--O0--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) main.c -o main library.a -I . -Os
	echo "\n\n------\n--Os--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	make clean

dynamic:
	$(CC) -fPIC -c library.c -o library.o
	$(CC) -fPIC -shared library.o -o library.so
	echo "\n\n------------------------------------\n|--------------DYNAMIC-------------|\n------------------------------------" >> $(FILE)
	$(CC) -o main main_dynamic.c -I . -ldl -O
	echo "-----\n--O--\n-----\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) -o main main_dynamic.c -I . -ldl -O1
	echo "\n\n------\n--O1--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) -o main main_dynamic.c -I . -ldl -O2
	echo "\n\n------\n--O2--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) -o main main_dynamic.c -I . -ldl -O3
	echo "\n\n------\n--O3--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) -o main main_dynamic.c -I . -ldl -O0
	echo "\n\n------\n--O0--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) -o main main_dynamic.c -I . -ldl -Os
	echo "\n\n------\n--Os--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	make clean


shared:
	$(CC) -fPIC -c library.c -o library.o
	$(CC) -fPIC -shared -o library.so library.o
	echo "\n\n------------------------------------\n|--------------SHARED--------------|\n------------------------------------" >> $(FILE)
	$(CC) main.c -o main -L. library.o -I . -O
	echo "-----\n--O--\n-----\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) main.c -o main -L. library.o -I . -O1
	echo "\n\n------\n--O1--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) main.c -o main -L. library.o -I . -O2
	echo "\n\n------\n--O2--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) main.c -o main -L. library.o -I . -O3
	echo "\n\n------\n--O3--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) main.c -o main -L. library.o -I . -O0
	echo "\n\n------\n--O0--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	$(CC) main.c -o main -L. library.o -I . -Os
	echo "\n\n------\n--Os--\n------\n" >> $(FILE)
	./main $(ARGUMENTS) >> $(FILE)
	make clean

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.a
