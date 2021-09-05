
test: bin/runtest
	gdb -q bin/runtest -ex r -ex quit

lib/capture.o: lib/capture.c
	gcc -g -I. -o lib/capture.o -c lib/capture.c

test/test.o: test/test.c
	gcc -g -I. -o test/test.o -c test/test.c

bin/runtest: test/test.o lib/capture.o
	gcc -g -I. -o bin/runtest lib/capture.o test/test.o

.PHONY: clean
clean:
	rm -f bin/* lib/*.o test/*.o
