CC = gcc -Wall
FILE1 = temp1.txt
FILE2 = temp2.txt
FILE = raport.txt

all: compile 1 4 512 4096 8192

compile:
	$(CC) -c library.c
	ar rcs library.a library.o
	$(CC) main.c -o main library.a -I .
	make clean


1:
	./main generate $(FILE1) 6000 1 | tee -a $(FILE)
	./main copy $(FILE1) $(FILE2) 6000 1 lib | tee -a $(FILE)
	./main sort $(FILE1) 6000 1 lib | tee -a $(FILE)
	./main sort $(FILE2) 6000 1 sys | tee -a $(FILE)
	echo "---------\n\n" | tee -a $(FILE)
	./main generate $(FILE1) 12000 1 | tee -a $(FILE)
	./main copy $(FILE1) $(FILE2) 12000 1 sys | tee -a $(FILE)
	./main sort $(FILE1) 12000 1 lib | tee -a $(FILE)
	./main sort $(FILE2) 12000 1 sys | tee -a $(FILE)
	echo "--------------------\n--------------------" | tee -a $(FILE)

	
4:
	./main generate $(FILE1) 5000 4 | tee -a $(FILE)
	./main copy $(FILE1) $(FILE2) 5000 4 lib | tee -a $(FILE)
	./main sort $(FILE1) 5000 4 lib | tee -a $(FILE)
	./main sort $(FILE2) 5000 4 sys | tee -a $(FILE)
	echo "---------\n\n" | tee -a $(FILE)
	./main generate $(FILE1) 10000 4 | tee -a $(FILE)
	./main copy $(FILE1) $(FILE2) 10000 4 sys | tee -a $(FILE)
	./main sort $(FILE1) 10000 4 lib | tee -a $(FILE)
	./main sort $(FILE2) 10000 4 sys | tee -a $(FILE)
	echo "--------------------\n--------------------" | tee -a $(FILE)

	
512:
	./main generate $(FILE1) 4000 512 | tee -a $(FILE)
	./main copy $(FILE1) $(FILE2) 4000 512 lib | tee -a $(FILE)
	./main sort $(FILE1) 4000 512 lib | tee -a $(FILE)
	./main sort $(FILE2) 4000 512 sys | tee -a $(FILE)
	echo "---------\n\n" | tee -a $(FILE)
	./main generate $(FILE1) 8000 512 | tee -a $(FILE)
	./main copy $(FILE1) $(FILE2) 8000 512 sys | tee -a $(FILE)
	./main sort $(FILE1) 8000 512 lib | tee -a $(FILE)
	./main sort $(FILE2) 8000 512 sys | tee -a $(FILE)
	echo "--------------------\n--------------------" | tee -a $(FILE)

	
4096:
	./main generate $(FILE1) 2000 4096 | tee -a $(FILE)
	./main copy $(FILE1) $(FILE2) 2000 4096 lib | tee -a $(FILE)
	./main sort $(FILE1) 2000 4096 lib | tee -a $(FILE)
	./main sort $(FILE2) 2000 4096 sys | tee -a $(FILE)
	echo "---------\n\n" | tee -a $(FILE)
	./main generate $(FILE1) 4000 4096 | tee -a $(FILE)
	./main copy $(FILE1) $(FILE2) 4000 4096 sys | tee -a $(FILE)
	./main sort $(FILE1) 4000 4096 lib | tee -a $(FILE)
	./main sort $(FILE2) 4000 4096 sys | tee -a $(FILE)
	echo "--------------------\n--------------------" | tee -a $(FILE)

	
8192:
	./main generate $(FILE1) 1500 8192 | tee -a $(FILE)
	./main copy $(FILE1) $(FILE2) 1500 8192 lib | tee -a $(FILE)
	./main sort $(FILE1) 1500 8192 lib | tee -a $(FILE)
	./main sort $(FILE2) 1500 8192 sys | tee -a $(FILE)
	echo "---------\n\n" | tee -a $(FILE)
	./main generate $(FILE1) 3000 8192 | tee -a $(FILE)
	./main copy $(FILE1) $(FILE2) 3000 8192 sys | tee -a $(FILE)
	./main sort $(FILE1) 3000 8192 lib | tee -a $(FILE)
	./main sort $(FILE2) 3000 8192 sys | tee -a $(FILE)
	echo "--------------------\n--------------------" | tee -a $(FILE)

clean:
	rm -f *.o
	rm -f *.a
