
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -O3
LIBNAME = eelf
SRCDIR = ./src
BUILDDIR = ./build

INCLUDES = ./include
OBJ = $(BUILDDIR)/elf.o

all: lib

## Generic compilation rule
$(OBJ): $(SRCDIR)/elf.c
	$(CC) $(CFLAGS) -c -fpic $(SRCDIR)/elf.c -o $@

lib: $(OBJ)
	$(CC) $(CFLAGS) -shared $(OBJ) -o $(BUILDDIR)/lib$(LIBNAME).so

test: lib
	$(CC) -L$(BUILDDIR) -l$(LIBNAME) -o $(BUILDDIR)/test $(SRCDIR)/test.c

clean:
	rm -rf $(BUILDDIR)/
