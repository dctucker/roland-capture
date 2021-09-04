
test: bin/runtest
	gdb -q bin/runtest -ex r -ex quit

bin/runtest: test/test.c lib/*.h
	gcc -g -I. test/test.c -o bin/runtest

.PHONY: clean
clean:
	rm -f bin/*
