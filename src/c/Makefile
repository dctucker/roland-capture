
test: bin/runtest
	bin/runtest

bin/runtest: test/test.c lib/*.h
	gcc -I. test/test.c -o bin/runtest

.PHONY: clean
clean:
	rm -f bin/*
