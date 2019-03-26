CC = gcc -Wall

program: compileMain run clean
test: compileTest makeTest clean

compileMain:
	$(CC) main.c -o main

compileTest:
	$(CC) tester.c -o tester

run:
	./main ./plik
	
makeTest:
	./tester ./plik1 1 3 100

clean:
	rm -f *.o
	rm -f main
	rm -f tester
