IDIR = .
ODIR = obj
CFLAGS=-I$(IDIR) -g
LIBS=-lasound -lm

OBJ = app/main.o app/comm.o lib/capture.o lib/roland.o lib/memory.o lib/types.o lib/mixer.o
DEPS = lib/$(patsubst %.c,%.o,$(wildcard filters/*.h)) $(wildcard *.h)

all: bin/main bin/test

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

bin/main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

bin/test: test/test.o lib/capture.o lib/roland.o lib/memory.o lib/types.o lib/mixer.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test: bin/test
	gdb -q bin/test -ex r -ex quit

.phony: clean run
run:
	bin/main

clean:
	rm -f bin/* **/*.o
