all: geto

geto:
	gcc -o test test.c

run: geto
	./test

clean:
	rm test
