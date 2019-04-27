GCC = gcc -Wall

compile: server.out client.out

server.out: server.o
	$(GCC) -o server.out server.c -lrt

server.o: server.c
	$(GCC) -c -o server server.c -lrt

client.out: client.o
	$(GCC) -o client.out client.c -lrt

client.o: client.c
	$(GCC) -c -o client client.c -lrt

run_server:
	./server.out

run_client:
	./client.out

clean:
	rm -f *.o *.out server client
