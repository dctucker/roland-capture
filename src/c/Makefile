IDIR = .
ODIR = obj
CFLAGS=-I$(IDIR) -g -save-temps
LIBS=-lasound -lm

OBJ = app/main.o app/comm.o lib/capture.o lib/roland.o lib/memory.o lib/types.o lib/mixer.o
DEPS = lib/$(patsubst %.c,%.o,$(wildcard filters/*.h)) $(wildcard *.h)

all: bin/main bin/test_main bin/test_main bin/test_mixer

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

test/%.o: test/%.c test/%.h
	$(CC) -c -o $@ $< $(CFLAGS)

bin/main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

bin/test_main: test/test_main.o lib/capture.o lib/roland.o lib/memory.o lib/types.o lib/mixer.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

bin/test_mixer: test/test_mixer.o lib/capture.o lib/roland.o lib/memory.o lib/types.o lib/mixer.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test: bin/test_main
	gdb -q bin/test_main -ex r -ex quit

.phony: clean run
run:
	bin/main

weigh:
	du -h **/*.o bin/**/* bin/* | sort -h

clean:
	rm -f bin/test_* bin/* **/*.o
