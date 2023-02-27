all: makeo makeexe

makeo:
	cd ./code/ && make

makeexe:
	gcc -I ./include/ ./code/main.c ./code/fibo.c -o fibo

clean:
	rm -rf ./code/*.o

