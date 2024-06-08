 
project4a: libcoro1.a  project4a.c lcoroutines.h
	gcc -g project4a.c -o project4a -lcoro1 -L. 

libcoro1.a: coroutines.o lcoroutines.h
	ar rcs libcoro1.a coroutines.o

coroutines.o: coroutines.c lcoroutines.h
	gcc -Wall -c coroutines.c -o coroutines.o


