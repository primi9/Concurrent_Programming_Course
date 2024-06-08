
project4b: libmythreads.a  project4b.c mythreads.h
	gcc -g project4b.c -o project4b -lmythreads -L.

libmythreads.a: mythreadslib.o mythreads.h
	ar rcs libmythreads.a mythreadslib.o

mythreadslib.o: mythreadslib.c mythreads.h
	gcc -Wall -c mythreadslib.c -o mythreadslib.o


