CC = gcc -Wall
ARGUMENTS = create_table 3 search_directory / \\* temp1 search_directory /usr \\* temp2 search_directory /usr/bin \\* temp3 remove_block 0 remove_block 1 remove_block 2
FILE = raport3b.txt

all: static dynamic shared

static:
	echo "------------------------------------\n|---------------STATIC-------------|\n------------------------------------" | tee -a $(FILE)
	
	$(CC) -c library.c -O
	ar rcs library.a library.o
	$(CC) main.c -o main library.a -I . -O
	echo "-----\n--O--\n-----\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -c library.c -O1
	ar rcs library.a library.o
	$(CC) main.c -o main library.a -I . -O1
	echo "\n\n------\n--O1--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -c library.c -O2
	ar rcs library.a library.o
	$(CC) main.c -o main library.a -I . -O2
	echo "\n\n------\n--O2--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -c library.c -O3
	ar rcs library.a library.o
	$(CC) main.c -o main library.a -I . -O3
	echo "\n\n------\n--O3--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -c library.c -O0
	ar rcs library.a library.o
	$(CC) main.c -o main library.a -I . -O0
	echo "\n\n------\n--O0--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -c library.c -Os
	ar rcs library.a library.o
	$(CC) main.c -o main library.a -I . -Os
	echo "\n\n------\n--Os--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean



dynamic:
	echo "\n\n------------------------------------\n|--------------DYNAMIC-------------|\n------------------------------------" | tee -a $(FILE)
	
	$(CC) -fPIC -c library.c -o library.o -O
	$(CC) -fPIC -shared library.o -o library.so
	$(CC) -o main main_dynamic.c -I . -ldl -O
	echo "-----\n--O--\n-----\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -fPIC -c library.c -o library.o -O1
	$(CC) -fPIC -shared library.o -o library.so
	$(CC) -o main main_dynamic.c -I . -ldl -O1
	echo "\n\n------\n--O1--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -fPIC -c library.c -o library.o -O2
	$(CC) -fPIC -shared library.o -o library.so
	$(CC) -o main main_dynamic.c -I . -ldl -O2
	echo "\n\n------\n--O2--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -fPIC -c library.c -o library.o -O3
	$(CC) -fPIC -shared library.o -o library.so
	$(CC) -o main main_dynamic.c -I . -ldl -O3
	echo "\n\n------\n--O3--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -fPIC -c library.c -o library.o -O0
	$(CC) -fPIC -shared library.o -o library.so
	$(CC) -o main main_dynamic.c -I . -ldl -O0
	echo "\n\n------\n--O0--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -fPIC -c library.c -o library.o -Os
	$(CC) -fPIC -shared library.o -o library.so
	$(CC) -o main main_dynamic.c -I . -ldl -Os
	echo "\n\n------\n--Os--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean




shared:
	echo "\n\n------------------------------------\n|--------------SHARED--------------|\n------------------------------------" | tee -a $(FILE)
	
	$(CC) -fPIC -c library.c -o library.o -O
	$(CC) -fPIC -shared -o library.so library.o
	$(CC) main.c -o main -L. library.o -I . -O
	echo "-----\n--O--\n-----\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -fPIC -c library.c -o library.o -O1
	$(CC) -fPIC -shared -o library.so library.o
	$(CC) main.c -o main -L. library.o -I . -O1
	echo "\n\n------\n--O1--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -fPIC -c library.c -o library.o -O2
	$(CC) -fPIC -shared -o library.so library.o
	$(CC) main.c -o main -L. library.o -I . -O2
	echo "\n\n------\n--O2--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -fPIC -c library.c -o library.o -O3
	$(CC) -fPIC -shared -o library.so library.o
	$(CC) main.c -o main -L. library.o -I . -O3
	echo "\n\n------\n--O3--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -fPIC -c library.c -o library.o -O0
	$(CC) -fPIC -shared -o library.so library.o
	$(CC) main.c -o main -L. library.o -I . -O0
	echo "\n\n------\n--O0--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean
	
	$(CC) -fPIC -c library.c -o library.o -Os
	$(CC) -fPIC -shared -o library.so library.o
	$(CC) main.c -o main -L. library.o -I . -Os
	echo "\n\n------\n--Os--\n------\n" | tee -a $(FILE)
	./main $(ARGUMENTS) | tee -a $(FILE)
	make clean

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.a
	rm -f main
