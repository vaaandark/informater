CC=clang
SANITIZE=-fsanitize=address
#CFLAGS=-std=c99 -Wall -Wextra -Werror -Wshadow $(SANITIZE) -g -O0
CFLAGS=-std=c99 -Wall -Wextra -Wshadow $(SANITIZE) -g -O0
IRREGEXSOURCE=src/irregex
SOURCEDIR=src
BUILDDIR=build
OBJECTS=$(BUILDDIR)/re.o $(BUILDDIR)/NFA.o $(BUILDDIR)/irregex.o \
		$(BUILDDIR)/lexer.o $(BUILDDIR)/parser.o $(BUILDDIR)/err.o

all: $(OBJECTS) $(BUILDDIR)/main.o $(BUILDDIR)/ast2graph.o
	$(CC) $(CFLAGS) $(OBJECTS) $(BUILDDIR)/main.o -o $(BUILDDIR)/informater
	$(CC) $(CFLAGS) $(OBJECTS) $(BUILDDIR)/ast2graph.o -o $(BUILDDIR)/ast2graph

$(BUILDDIR)/re.o: $(IRREGEXSOURCE)/def.h
	$(CC) $(CFLAGS) -c $(IRREGEXSOURCE)/re.c -o $(BUILDDIR)/re.o

$(BUILDDIR)/NFA.o: $(IRREGEXSOURCE)/re.h
	$(CC) $(CFLAGS) -c $(IRREGEXSOURCE)/NFA.c -o $(BUILDDIR)/NFA.o

$(BUILDDIR)/irregex.o: $(IRREGEXSOURCE)/NFA.h
	$(CC) $(CFLAGS) -c $(IRREGEXSOURCE)/irregex.c -o $(BUILDDIR)/irregex.o

$(BUILDDIR)/lexer.o: $(IRREGEXSOURCE)/irregex.h
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/lexer.c -o $(BUILDDIR)/lexer.o

$(BUILDDIR)/err.o:
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/err.c -o $(BUILDDIR)/err.o

$(BUILDDIR)/parser.o: $(SOURCEDIR)/lexer.h $(SOURCEDIR)/def.h $(SOURCEDIR)/err.h
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/parser.c -o $(BUILDDIR)/parser.o

$(BUILDDIR)/main.o: $(SOURCEDIR)/parser.h
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/main.c -o $(BUILDDIR)/main.o

$(BUILDDIR)/ast2graph.o: $(SOURCEDIR)/parser.h
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/ast2graph.c -o $(BUILDDIR)/ast2graph.o

.PHONY: clean
clean:
	-rm -rf $(BUILDDIR)/*

