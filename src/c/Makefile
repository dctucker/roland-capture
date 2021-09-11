IDIR = .
ODIR = obj
CFLAGS=-I$(IDIR) -fvisibility=hidden
# -g -save-temps
LIBS=-lasound -lm

OBJDIR = obj
VPATH = $(OBJDIR)
OBJ = lib/capmix.o lib/comm.o lib/capture.o lib/roland.o lib/memory.o lib/types.o
DEPS = lib/$(patsubst %.c,%.o,$(wildcard filters/*.h)) $(wildcard *.h)

all: bin/main bin/test_main bin/test_main bin/test_mixer

%.o: %.c %.h
	$(CC) -c -o $(OBJDIR)/$@ $< $(CFLAGS) -fPIC

test/%.o: test/%.c test/%.h
	$(CC) -c -o $(OBJDIR)/$@ $< $(CFLAGS)

bin/test_main: $(OBJ) test/test_main.o
	$(CC) $(CFLAGS) $(LIBS) -o $(OBJDIR)/$@ $(addprefix $(OBJDIR)/,$(OBJ)) test/test_main.c

bin/test_mixer: $(OBJ) lib/mixer.o test/test_mixer.o
	$(CC) $(CFLAGS) $(LIBS) -o $(OBJDIR)/$@ $(OBJDIR)/lib/mixer.o test/test_mixer.c

lib/libcapmix.so: $(OBJ)
	$(CC) $(LIBS) $(CFLAGS) -fPIC -s -shared -Wl,-soname,libcapmix.so.1 -o $(OBJDIR)/lib/libcapmix.so.1.0.0 -lc $(addprefix $(OBJDIR)/,$(OBJ))
	cd $(OBJDIR)/lib ; ln -sf libcapmix.so.1.0.0 libcapmix.so.1
	cd $(OBJDIR)/lib ; ln -sf libcapmix.so.1.0.0 libcapmix.so

bin/main: lib/libcapmix.so
	$(CC) $(CFLAGS) $(LIBS) -o $(OBJDIR)/$@ app/main.c -L$(OBJDIR)/lib -lcapmix '-Wl,-R,$$ORIGIN/../lib'


#libcapmix.a: lib/capmix.o lib/comm.o lib/capture.o lib/roland.o lib/memory.o lib/types.o
#	#$(CC) $(CFLAGS) -c $^
#	#$(CC) $(LIBS) $(CFLAGS) -c lib/capmix.c lib/comm.c lib/capture.c lib/roland.c lib/memory.c lib/types.c
#	#strip --strip-unneeded $^
#	ld -Ur -o libcapmix.o $^
#	objcopy --strip-all --keep-symbols api.txt -R .note -R .comment libcapmix.o libcapmix-s.o
#	$(AR) rcs "$(LIBS)" $@ libcapmix-s.o
#bin/main: libcapmix.a app/main.o # static library
#	$(CC) $(LIBS) $(CFLAGS) -o $@ app/main.o -L. -lcapmix

.phony: clean run test
run:
	bin/main

test: bin/test_main
	gdb -q bin/test_main -ex r -ex quit

weigh:
	du -h **/*.o bin/**/* bin/* *.a | sort -h

clean:
	rm -f obj/**/* bin/test_* bin/* **/*.o **/*.so* *.a *.o *.so* **/*.s **/*.i
