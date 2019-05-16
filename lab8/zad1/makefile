GCC = gcc -Wall
F = ./files/
WYNIK = ./wynik.txt

compile:
	$(GCC) -o main main.c -lm -lpthread

run:
	./main 1 block $(F)1.pgm $(F)3x3.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 1 block $(F)1.pgm $(F)15x15.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 1 block $(F)1.pgm $(F)60x60.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 1 interleaved $(F)1.pgm $(F)3x3.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 1 interleaved $(F)1.pgm $(F)15x15.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 1 interleaved $(F)1.pgm $(F)60x60.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 2 block $(F)1.pgm $(F)3x3.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 2 block $(F)1.pgm $(F)15x15.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 2 block $(F)1.pgm $(F)60x60.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 2 interleaved $(F)1.pgm $(F)3x3.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 2 interleaved $(F)1.pgm $(F)15x15.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 2 interleaved $(F)1.pgm $(F)60x60.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 4 block $(F)1.pgm $(F)3x3.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 4 block $(F)1.pgm $(F)15x15.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 4 block $(F)1.pgm $(F)60x60.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 4 interleaved $(F)1.pgm $(F)3x3.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 4 interleaved $(F)1.pgm $(F)15x15.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 4 interleaved $(F)1.pgm $(F)60x60.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 8 block $(F)1.pgm $(F)3x3.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 8 block $(F)1.pgm $(F)15x15.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 8 block $(F)1.pgm $(F)60x60.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 8 interleaved $(F)1.pgm $(F)3x3.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 8 interleaved $(F)1.pgm $(F)15x15.trans $(F)wynik1.pgm >> $(WYNIK)
	./main 8 interleaved $(F)1.pgm $(F)60x60.trans $(F)wynik1.pgm >> $(WYNIK)

clean:
	rm -f main
