# Apps Makefile

BINS=pagerank
OBJS=vec.o web.o algorithm.o comm.o
LIBNAME=libhiredis

# Fallback to gcc when $CC is not in $PATH.
CC:=gcc
OPT?=-O3
WARN=-Wall -W -Wstrict-prototypes -Wwrite-strings
DEBUG?= -g -ggdb

REAL_CFLAGS=$(OPT) -fPIC $(CFLAGS) $(WARN) -D_GNU_SOURCE $(DEBUG)
REAL_LDFLAGS=$(LDFLAGS)

DYLIBNAME=./lib/$(LIBNAME).so
STLIBNAME=./lib/$(LIBNAME).a

all: $(BINS)

DEP=$(STLIBNAME)

# Binaries:
pagerank: $(OBJS) pagerank.o $(DEP)
	$(CC) -o $@ $(REAL_LDFLAGS) $^ -lm -lpthread 

#test: test.o $(DEP)
#	$(CC) -o $@ $(REAL_LDFLAGS) $^ -lm -lpthread 

.c.o:
	$(CC) -std=c99 -pedantic -c $(REAL_CFLAGS) $< -lm

tags:
	ctags -R .
	
clean:
	rm -rf $(BINS) $(OBJS) *.o

dep:
	$(CC) -MM *.c

32bit:
	@echo ""
	@echo "WARNING: if this fails under Linux you probably need to install libc6-dev-i386"
	@echo ""
	$(MAKE) CFLAGS="-m32" LDFLAGS="-m32"

gprof:
	$(MAKE) CFLAGS="-pg" LDFLAGS="-pg"

gcov:
	$(MAKE) CFLAGS="-fprofile-arcs -ftest-coverage" LDFLAGS="-fprofile-arcs"

noopt:
	$(MAKE) OPT=""

.PHONY: all clean dep 32bit gprof gcov noopt
