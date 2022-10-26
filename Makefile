IDIR = .
ODIR = obj
CFLAGS=-I$(IDIR) -fvisibility=hidden -g -save-temps
LIBS=-lasound -lm
STRIP=-s

OBJDIR = obj
VPATH = $(OBJDIR)
OBJ = lib/capmix.o lib/comm.o lib/capture.o lib/roland.o lib/memory.o lib/types.o
DEPS = lib/$(patsubst %.c,%.o,$(wildcard types/*.h)) $(wildcard *.h)

LCAPMIX = -L$(OBJDIR)/lib -lcapmix '-Wl,-R,$$ORIGIN/../lib'
TESTS = bin/test_memory bin/test_mixer bin/test_capture bin/test_types
BINARIES = bin/main bin/meters bin/list-memory bin/capmixer $(TESTS)

all: $(BINARIES)

tests: $(TESTS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/$@ $< $(CFLAGS) -fPIC

test/%.o: test/%.c test/%.h
	$(CC) -c -o $(OBJDIR)/$@ $< $(CFLAGS)

#bin/test_main: test/test.h $(OBJ) test/test_main.c
#	$(CC) $(CFLAGS) $(LIBS) -o $(OBJDIR)/$@ $(addprefix $(OBJDIR)/,$(OBJ)) test/test_main.c

bin/test_mixer: test/test.h $(OBJ) lib/mixer.o test/test_mixer.c
	$(CC) $(CFLAGS) $(LIBS) -o $(OBJDIR)/$@ $(OBJDIR)/lib/mixer.o test/test_mixer.c

bin/test_memory: test/test.h $(OBJ) lib/memory.o test/test_memory.c
	$(CC) $(CFLAGS) $(LIBS) -o $(OBJDIR)/$@ $(OBJDIR)/lib/memory.o test/test_memory.c

bin/test_capture: test/test.h $(OBJ) lib/capture.o test/test_capture.c
	$(CC) $(CFLAGS) $(LIBS) -o $(OBJDIR)/$@ $(OBJDIR)/lib/types.o $(OBJDIR)/lib/capture.o test/test_capture.c

bin/test_types: test/test.h $(OBJ) lib/types.o test/test_types.o
	$(CC) $(CFLAGS) $(LIBS) -o $(OBJDIR)/$@ $(OBJDIR)/lib/types.o test/test_types.c

lib/libcapmix.so: $(OBJ)
	$(CC) $(LIBS) $(CFLAGS) -fPIC $(STRIP) -shared -Wl,-soname,libcapmix.so.1 -o $(OBJDIR)/lib/libcapmix.so.1.0.0 -lc $(addprefix $(OBJDIR)/,$(OBJ))
	cd $(OBJDIR)/lib ; ln -sf libcapmix.so.1.0.0 libcapmix.so.1
	cd $(OBJDIR)/lib ; ln -sf libcapmix.so.1.0.0 libcapmix.so

#bin/main: lib/libcapmix.so
#	$(CC) $(CFLAGS) $(LIBS) -o $(OBJDIR)/$@ app/main.c $(LCAPMIX)

bin/capmixer-dynamic: lib/libcapmix.so lib/mixer.o app/capmixer.c
	$(CC) $(CFLAGS) -lmenu -lncurses -o $@ $(OBJDIR)/lib/mixer.o app/capmixer.c $(LCAPMIX)

libcapmix.a:
	#$(CC) $(CFLAGS) -c $^
	#$(CC) $(LIBS) $(CFLAGS) -c lib/capmix.c lib/comm.c lib/capture.c lib/roland.c lib/memory.c lib/types.c
	#strip --strip-unneeded $^
	$(LD) -Ur -o $(OBJDIR)/lib/libcapmix.o  $(addprefix $(OBJDIR)/,lib/capmix.o lib/comm.o lib/capture.o lib/roland.o lib/memory.o lib/types.o)
	objcopy --strip-all --keep-symbols=api.txt -R .note -R .comment -R eh_frame $(OBJDIR)/lib/libcapmix.o $(OBJDIR)/lib/libcapmix-s.o
	$(AR) rcs "$(LIBS)" $(OBJDIR)/lib/libcapmix.a $(OBJDIR)/lib/libcapmix-s.o

bin/meters: $(OBJ) app/meters.c
	$(CC) $(LIBS) $(CFLAGS) -o bin/meters $(addprefix $(OBJDIR)/,$(OBJ)) app/meters.c

bin/list-memory: $(OBJ) app/list-memory.c
	$(CC) $(LIBS) $(CFLAGS) -o bin/list-memory $(addprefix $(OBJDIR)/,$(OBJ)) app/list-memory.c

bin/main: $(OBJ) app/main.c
	$(CC) $(LIBS) $(CFLAGS) -o bin/main $(addprefix $(OBJDIR)/,$(OBJ)) app/main.c
	cd bin && ln -sf main octa && ln -sf main studio && cd -
	# -L$(OBJDIR)/lib -lcapmix

bin/main-dynamic: $(OBJ) app/main.c
	$(CC) $(CFLAGS) -o $@ app/main.c $(LCAPMIX)

#bin/main: $(OBJ) app/main.c # static library
#	$(CC) $(LIBS) $(CFLAGS) -o bin/main app/main.c
#	# -L$(OBJDIR)/lib -lcapmix

bin/capmixer-static: $(OBJ) lib/mixer.o app/capmixer.c
	$(CC) $(CFLAGS) $(LIBS) -lmenu -lncurses -o $@ $(addprefix $(OBJDIR)/,$(OBJ)) $(OBJDIR)/lib/mixer.o app/capmixer.c

bin/capmixer: bin/capmixer-static bin/capmixer-dynamic

.PHONY: clean run test doc
doc:
	doxygen doc/doxygen.cfg

run:
	bin/main

debug: $(TESTS)
	for f in $^; do gdb -q $$f -ex r -ex quit; done

test: $(TESTS)
	for f in $^; do $$f; done

weigh:
	du -h obj/**/* **/*.o bin/**/* bin/* *.a | sort -h

clean:
	rm -rf doc/html
	rm -f obj/**/* bin/test_* bin/* **/*.o **/*.so* *.a *.o *.so* **/*.s **/*.i
