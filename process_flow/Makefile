processflow: processo.o
	gcc processo.o -o processflow

processo.o: processo.c
	gcc -c processo.c

clean:
	rm -f *.o processflow

.PHONY: clean