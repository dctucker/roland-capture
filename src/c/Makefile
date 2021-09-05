IDIR = .
ODIR = obj
CFLAGS=-I$(IDIR) -g -fcommon
LIBS=-lasound

OBJ = app/main.o app/comm.o lib/capture.o lib/roland.o

all: bin/main bin/test

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

bin/main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

bin/test: test/test.o lib/capture.o
	$(CC) -o $@ $^ $(CFLAGS)

test: bin/test
	gdb -q bin/runtest -ex r -ex quit

.phony: clean
clean:
	rm -f bin/* **/*.o
