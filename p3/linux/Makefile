
libmem.so: mem.o
	gcc -g -shared -o libmem.so mem.o;\
	LD_LIBRARY_PATH=.;\
	export LD_LIBRARY_PATH;\

mem.o: mem.c mem.h
	gcc -g -c -fpic -o mem.o mem.c

myprogram: main.c
	gcc -lmem -L. -o main.o main.c -Wall -Werror

random: random.c
	gcc -lmem -L. -o random.o random.c -Wall -Werror -g

nospace: nospace.c
	gcc -lmem -L. -o nospace.o nospace.c -Wall -Werror -g

clean:
	rm -rf *.o libmem.so


