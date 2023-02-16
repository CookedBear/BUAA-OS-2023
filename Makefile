.PHONY: clean

out: calc case_all
	gcc -o calc calc.c
	./calc < case_all > out

case_add: case_gen
	./casegen add 100 > case_add

case_sub: case_gen
	./casegen sub 100 > case_sub

case_mul: case_gen
	./casegen mul 100 > case_mul

case_div: case_gen
	./casegen div 100 > case_div

case_gen: 
	gcc -o casegen casegen.c

case_all: case_add case_sub case_mul case_div
	cat case_add case_sub case_mul case_div > case_all

# Your code here.

clean:
	rm -f out calc casegen case_*

